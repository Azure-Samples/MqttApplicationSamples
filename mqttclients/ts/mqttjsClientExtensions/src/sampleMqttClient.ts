import { Logger } from './logger';
import { MqttConnectionSettings } from './mqttConnectionSettings';
import {
    ErrorWithReasonCode,
    IConnackPacket,
    IDisconnectPacket,
    IPublishPacket,
    MqttClient,
    connect as mqttConnect
} from 'mqtt';
import { QoS } from 'mqtt-packet';

const ModuleName = 'SampleMqttClient';

export class SampleMqttClient {
    private mqttClientInternal: MqttClient;
    private connectionSettings: MqttConnectionSettings;

    constructor(connectionSettings: MqttConnectionSettings) {
        this.connectionSettings = connectionSettings;
    }

    public get mqttClient() {
        return this.mqttClientInternal;
    }

    public get connected(): boolean {
        return this.mqttClientInternal?.connected || false;
    }

    public async endClientSession(): Promise<void> {
        await this.mqttClientInternal.endAsync(true);
    }

    public async connect(): Promise<void> {
        try {
            Logger.log([ModuleName, 'info'], `Initializing MQTT client`);

            this.mqttClientInternal = mqttConnect(MqttConnectionSettings.createMqttClientOptions(this.connectionSettings));

            // Attach MQTT client event handlers
            this.mqttClientInternal.on('connect', this.onConnect.bind(this));
            this.mqttClientInternal.on('disconnect', this.onDisconnect.bind(this));
            this.mqttClientInternal.on('message', this.onMessage.bind(this));
            this.mqttClientInternal.on('error', this.onError.bind(this));
            this.mqttClientInternal.on('close', this.onClose.bind(this));
            this.mqttClientInternal.on('end', this.onEnd.bind(this));
            this.mqttClientInternal.on('reconnect', this.onReconnect.bind(this));
            this.mqttClientInternal.on('offline', this.onOffline.bind(this));

            // Connect to MQTT broker
            Logger.log([ModuleName, 'info'], `Starting connection for clientId: ${this.mqttClientInternal.options.clientId}`);

            this.mqttClientInternal.connect();

            await new Promise<void>((resolve) => {
                const interval = setInterval(() => {
                    if (this.mqttClientInternal.connected) {
                        clearInterval(interval);

                        return resolve();
                    }
                }, 1000);
            });

            if (!this.mqttClientInternal.connected) {
                await this.mqttClientInternal.endAsync(true);

                throw new Error('Unable to connect to MQTT broker');
            }

            Logger.log([ModuleName, 'info'], `MQTT client connected - clientId: ${this.mqttClientInternal.options.clientId}`);
        }
        catch (ex) {
            Logger.log([ModuleName, 'error'], `MQTT client connect error: ${ex.message}`);
        }
    }

    public async subscribe(topic: string, qos: QoS): Promise<void> {
        try {
            Logger.log([ModuleName, 'info'], `Subscribing to MQTT topics: ${topic}`);

            await this.mqttClientInternal.subscribeAsync(topic, {
                qos
            });
        }
        catch (ex) {
            Logger.log([ModuleName, 'error'], `MQTT client subscribe error: ${ex.message}`);
        }
    }

    public async publish(topic: string, payload: string): Promise<void> {
        try {
            Logger.log([ModuleName, 'info'], `Publishing to MQTT topic: ${topic}, with payload: ${payload}`);

            await this.mqttClientInternal.publishAsync(topic, payload);
        }
        catch (ex) {
            Logger.log([ModuleName, 'error'], `MQTT client publish error: ${ex.message}`);
        }
    }

    private onConnect(_packet: IConnackPacket): void {
        Logger.log([ModuleName, 'info'], `Connected to MQTT broker: ${this.mqttClientInternal.options.host}:${this.mqttClientInternal.options.port}`);
    }

    private onDisconnect(_packet: IDisconnectPacket): void {
        Logger.log([ModuleName, 'info'], 'Disconnected from MQTT broker');
    }

    private onMessage(topic: string, payload: Buffer, _packet: IPublishPacket): void {
        Logger.log([ModuleName, 'info'], `Received message on topic: ${topic}, with payload: ${payload.toString('utf8')}`);
    }

    private onClose(): void {
        Logger.log([ModuleName, 'info'], 'MQTT broker connection closed');
    }

    private onEnd(): void {
        Logger.log([ModuleName, 'info'], 'MQTT broker connection ended');
    }

    private onReconnect(): void {
        Logger.log([ModuleName, 'info'], 'MQTT broker session re-connected');
    }

    private onOffline(): void {
        Logger.log([ModuleName, 'info'], 'MQTT broker connection is offline');
    }

    private onError(error: Error | ErrorWithReasonCode): void {
        Logger.log([ModuleName, 'error'], `MQTT client error:`);

        if ((error as ErrorWithReasonCode)?.code) {
            Logger.log([ModuleName, 'error'], `  - reason code: ${(error as ErrorWithReasonCode).code}`);

            if ((error as ErrorWithReasonCode)?.message) {
                Logger.log([ModuleName, 'error'], `  - message: ${(error as ErrorWithReasonCode).message}`);
            }
            else {
                const errors = (error as any).errors;
                if (errors && Array.isArray(errors)) {
                    for (const subError of errors) {
                        Logger.log([ModuleName, 'error'], `  - message: ${subError.message}`);
                    }
                }
            }
        }

        Logger.log([ModuleName, 'error'], `Terminating the sample...`);

        process.exit(1);
    }
}
