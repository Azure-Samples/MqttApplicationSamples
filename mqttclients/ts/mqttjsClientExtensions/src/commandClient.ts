import {
    IPublishPacket,
    MqttClient
} from 'mqtt';
import { v4 as uuidV4 } from 'uuid';
import {
    logger,
    DeferredPromise,
    IMessageSerializer
} from './';

const ModuleName = 'CommandClient';

export class CommandClient<T, TResp>
{
    private mqttClient: MqttClient;
    private requestTopic: string;
    private responseTopic: string;
    private commandName: string;
    private serializer: IMessageSerializer;
    private correlationId: Buffer;
    private deferredPromise: DeferredPromise<TResp>;

    constructor(mqttClient: MqttClient, requestTopicPattern: string, responseTopicPattern: string, commandName: string, serializer: IMessageSerializer) {
        this.mqttClient = mqttClient;
        this.commandName = commandName;
        this.serializer = serializer;

        this.requestTopic = requestTopicPattern.replace('{clientId}', mqttClient.options.clientId ?? '').replace('{commandName}', this.commandName);
        this.responseTopic = responseTopicPattern.replace('{clientId}', mqttClient.options.clientId ?? '').replace('{commandName}', this.commandName);

        this.mqttClient.on('message', async (topic: string, payload: Buffer, packet: IPublishPacket) => {
            logger.info({ tags: [ModuleName] }, `Message received on topic: ${topic} with mid: ${packet.messageId}`);

            if (topic === this.responseTopic) {
                if (packet.properties?.contentType !== serializer.contentType) {
                    throw new Error(`Invalid content type. Expected :${this.serializer.contentType} Actual :${packet.properties?.contentType}`);
                }
            }

            if (packet.properties?.correlationData?.compare(this.correlationId) !== 0) {
                throw new Error(`correlationId does not match. Expected ${this.correlationId} actual ${packet.properties?.correlationData?.toString()}`);
            }

            const response = this.serializer.fromBytes<TResp>(payload);

            return this.deferredPromise.resolve(response);
        });
    }

    public async invokeAsync(clientId: string, request: T, timeoutInMilliSeconds = 5000) {
        this.correlationId = Buffer.from(uuidV4());

        await this.mqttClient.subscribeAsync(this.responseTopic, {
            qos: 1,
        });

        const respBytes = this.serializer.toBytes(request);
        const pubAck = await this.mqttClient.publishAsync(this.requestTopic, respBytes, {
            qos: 1,
            properties: {
                contentType: this.serializer.contentType,
                correlationData: this.correlationId,
                responseTopic: this.responseTopic,
                userProperties: {
                    status: '200'
                }
            }
        });

        this.deferredPromise = new DeferredPromise<TResp>();

        return this.deferredPromise.promise;
    }
}
