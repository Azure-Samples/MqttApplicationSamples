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
    private commandName: string;
    private requestTopicPattern: string;
    private requestTopic: string;
    private responseTopicPattern: string;
    private responseTopic: string;
    private serializer: IMessageSerializer;
    private correlationId: string;
    private deferredPromise: DeferredPromise<TResp>;

    constructor(mqttClient: MqttClient, requestTopicPattern: string, responseTopicPattern: string, commandName: string, serializer: IMessageSerializer) {
        this.mqttClient = mqttClient;
        this.commandName = commandName;
        this.requestTopicPattern = requestTopicPattern;
        this.responseTopicPattern = responseTopicPattern;
        this.serializer = serializer;

        this.mqttClient.on('message', (topic: string, payload: Buffer, packet: IPublishPacket) => {
            if (topic === this.responseTopic) {
                if (packet.properties?.contentType !== serializer.contentType) {
                    logger.error({ tags: [ModuleName] }, `Message received on topic ${topic} but with invalid content type. Expected ${this.serializer.contentType} - received ${packet.properties?.contentType}`);
                }

                const responseCorrelationId = (packet.properties?.correlationData ?? '').toString();

                if (responseCorrelationId !== this.correlationId) {
                    logger.error({ tags: [ModuleName] }, `Message received on topic ${topic} but correlationId does not match. Expected ${this.correlationId} - received ${responseCorrelationId}`);
                }

                logger.info({ tags: [ModuleName] }, `Message received on topic: ${topic} with correlationId: ${responseCorrelationId}`);

                const response = this.serializer.fromBytes<TResp>(payload);

                return this.deferredPromise.resolve(response);
            }
        });
    }

    public async invokeAsync(clientId: string, request: T, timeoutInMilliSeconds = 5000): Promise<TResp> {
        this.requestTopic = this.requestTopicPattern.replace('{clientId}', clientId).replace('{commandName}', this.commandName);
        this.responseTopic = this.responseTopicPattern.replace('{clientId}', clientId).replace('{commandName}', this.commandName);

        this.correlationId = uuidV4();

        await this.mqttClient.subscribeAsync(this.responseTopic, {
            qos: 1,
        });

        const respBytes = this.serializer.toBytes(request);
        const pubAck = await this.mqttClient.publishAsync(this.requestTopic, respBytes, {
            qos: 1,
            properties: {
                contentType: this.serializer.contentType,
                correlationData: Buffer.from(this.correlationId),
                responseTopic: this.responseTopic,
                userProperties: {
                    status: '200'
                }
            }
        });

        logger.info({ tags: [ModuleName] }, `Published command request on topic: ${this.requestTopic}, with mid: ${pubAck?.messageId}, correlationId: ${this.correlationId}`);

        this.deferredPromise = new DeferredPromise<TResp>();
        return this.promiseWithTimeout<TResp>(this.deferredPromise.promise, timeoutInMilliSeconds);
    }

    private async promiseWithTimeout<T>(promise: Promise<T>, timeoutInMilliSeconds: number): Promise<T> {
        const timeout = new Promise<never>((_, reject) => {
            setTimeout(() => {
                reject(new Error(`Command response timed out after ${timeoutInMilliSeconds} milliseconds`));
            }, timeoutInMilliSeconds);
        });

        return Promise.race<T>([promise, timeout]);
    }
}
