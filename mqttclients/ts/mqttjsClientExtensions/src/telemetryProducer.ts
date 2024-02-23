import {
    IPublishPacket,
    MqttClient
} from 'mqtt';
import {
    QoS
} from 'mqtt-packet';
import { IMessageSerializer } from './';

export class TelemetryProducer<T>
{
    private mqttClient: MqttClient;
    private telemetryTopic: string;
    private serializer: IMessageSerializer;

    constructor(mqttClient: MqttClient, serializer: IMessageSerializer, topicPattern: string) {
        this.mqttClient = mqttClient;
        this.serializer = serializer;
        this.telemetryTopic = topicPattern.replace('{clientId}', mqttClient.options.clientId!);
    }

    public async SendTelemetryAsync(message: T, qos: QoS = 1, retain = false): Promise<IPublishPacket> {
        const pubAck = await this.mqttClient.publishAsync(this.telemetryTopic, this.serializer.toBytes(message), {
            qos,
            retain
        });

        return pubAck as IPublishPacket;
    }
}
