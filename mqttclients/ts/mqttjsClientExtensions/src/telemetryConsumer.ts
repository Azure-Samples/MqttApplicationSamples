import { MqttClient } from 'mqtt';


export class TelemetryConsumer<T> {
    private mqttClient: MqttClient;

    public constructor(mqttClient: MqttClient) {
        this.mqttClient = mqttClient;
    }

    public get foo(): T {
        return {} as T;
    }
}