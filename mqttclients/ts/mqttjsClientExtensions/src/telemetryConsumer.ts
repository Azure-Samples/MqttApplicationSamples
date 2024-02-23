import {
    IPublishPacket,
    MqttClient
} from 'mqtt';
import { matches as mqttMatches } from 'mqtt-pattern';
import {
    IMessageSerializer,
    TelemetryMessage
} from './';

export class TelemetryConsumer<T> {
    private mqttClient: MqttClient;
    private serializer: IMessageSerializer;
    private topicPattern: string;
    public onTelemetryReceived: (msg: TelemetryMessage<T>) => Promise<void>;

    public constructor(mqttClient: MqttClient, serializer: IMessageSerializer, topicPattern: string) {
        this.mqttClient = mqttClient;
        this.serializer = serializer;
        this.topicPattern = topicPattern;
    }

    public async startAsync(): Promise<void> {
        this.mqttClient.on('message', (topic: string, payload: Buffer, _packet: IPublishPacket) => {
            if (mqttMatches(this.topicPattern, topic)) {
                const segments = topic.split('/');
                const msg = new TelemetryMessage<T>(segments[1], this.serializer.fromBytes<T>(payload));

                void this.onTelemetryReceived(msg);
            }
        });

        await this.mqttClient.subscribeAsync(this.topicPattern, { qos: 1 });
    }
}