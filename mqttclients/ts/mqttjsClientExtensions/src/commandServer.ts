import {
    IPublishPacket,
    MqttClient
} from 'mqtt';
import {
    logger,
    IMessageSerializer
} from './';

const ModuleName = 'CommandServer';

export class CommandServer<T, TResp>
{
    private mqttClient: MqttClient;
    private requestTopic = '';
    private commandName: string;
    private serializer: IMessageSerializer;
    public onCommandReceived: (request: T) => TResp;

    constructor(mqttClient: MqttClient, requestTopicPattern: string, commandName: string, serializer: IMessageSerializer) {
        this.mqttClient = mqttClient;
        this.commandName = commandName;
        this.serializer = serializer;

        this.requestTopic = requestTopicPattern.replace('{clientId}', mqttClient.options.clientId ?? '').replace('{commandName}', this.commandName);

        this.mqttClient.on('message', async (topic: string, payload: Buffer, packet: IPublishPacket) => {
            logger.info({ tags: [ModuleName] }, `Message received on topic: ${topic} with mid: ${packet.messageId}`);

            if (topic === this.requestTopic) {
                if (packet.properties?.contentType !== serializer.contentType) {
                    throw new Error(`Invalid content type. Expected :${this.serializer.contentType} Actual :${packet.properties?.contentType}`);
                }

                const request = this.serializer.fromBytes<T>(payload);
                const responseTopic = packet.properties.responseTopic ?? '';

                logger.info({ tags: [ModuleName] }, `Sending response on topic: ${responseTopic}`);

                try {
                    const response = this.onCommandReceived(request);
                    const respBytes = this.serializer.toBytes(response);

                    const pubAck = await this.mqttClient.publishAsync(responseTopic, respBytes, {
                        qos: 1,
                        properties: {
                            contentType: this.serializer.contentType,
                            correlationData: packet.properties?.correlationData,
                            userProperties: {
                                status: '200'
                            }
                        }
                    });

                    logger.info({ tags: [ModuleName] }, `Published success response with mid: ${pubAck?.messageId} on topic: ${responseTopic}`);
                }
                catch (ex) {
                    const pubAck = await this.mqttClient.publishAsync(responseTopic, Buffer.from(ex.message), {
                        qos: 1,
                        properties: {
                            correlationData: packet.properties?.correlationData,
                            userProperties: {
                                status: '500'
                            }
                        }
                    });

                    logger.warn({ tags: [ModuleName] }, `Published error response with mid: ${pubAck?.messageId} on topic: ${responseTopic}`);
                }
            }
        });
    }

    public async startAsync(): Promise<void> {
        await this.mqttClient.subscribeAsync(this.requestTopic, {
            qos: 1
        });
    }
}