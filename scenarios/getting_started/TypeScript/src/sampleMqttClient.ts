import { Logger } from './logger';
import { ConnectionSettings } from './connectionSettings';
import {
    IClientOptions,
    MqttClient,
    connect as mqttConnect
} from 'mqtt';

const ModuleName = 'sampleMqttClient';

export class SampleMqttClient {
    private connectionSettings: ConnectionSettings;
    private mqttClient: MqttClient;

    constructor(connectionSettings: ConnectionSettings) {
        this.connectionSettings = connectionSettings;
    }

    public get connected(): boolean {
        return this.mqttClient?.connected || false;
    }

    public async connect(): Promise<void> {
        if (!this.connectionSettings.MQTT_CLEAN_SESSION) {
            throw new Error('This sample does not support connecting with existing sessions');
        }

        Logger.log([ModuleName, 'info'], `Initializing MQTT client`);
        const mqttClientOptions: IClientOptions = this.createMqttClientOptions();

        mqttClientOptions.manualConnect = true;
        this.mqttClient = mqttConnect(this.connectionSettings.MQTT_HOST_NAME, mqttClientOptions);

        // Attach MQTT client event handlers
        this.mqttClient.on('connect', this.onConnect.bind(this));
        this.mqttClient.on('disconnect', this.onDisconnect.bind(this));
        this.mqttClient.on('message', this.onMessage.bind(this));
        this.mqttClient.on('error', this.onError.bind(this));
        this.mqttClient.on('close', this.onClose.bind(this));
        this.mqttClient.on('end', this.onEnd.bind(this));
        this.mqttClient.on('reconnect', this.onReconnect.bind(this));
        this.mqttClient.on('offline', this.onOffline.bind(this));

        // Connect to MQTT broker
        Logger.log([ModuleName, 'info'], `Starting connection for clientId: ${this.connectionSettings.MQTT_CLIENT_ID}`);
        this.mqttClient.connect();

        await new Promise<void>((resolve) => {
            const interval = setInterval(() => {
                if (this.mqttClient.connected) {
                    clearInterval(interval);

                    return resolve();
                }
            }, 1000);
        });

        if (!this.mqttClient.connected) {
            throw new Error('Unable to connect to MQTT broker');
        }
    }

    private createMqttClientOptions(): IClientOptions {
        const mqttClientOptions: IClientOptions = {
            clientId: this.connectionSettings.MQTT_CLIENT_ID,
            clean: this.connectionSettings.MQTT_CLEAN_SESSION,
            protocolVersion: 3,
            protocol: 'tcp'
        };

        if (this.connectionSettings.MQTT_USERNAME) {
            mqttClientOptions.username = this.connectionSettings.MQTT_USERNAME;
            mqttClientOptions.password = this.connectionSettings.MQTT_PASSWORD;
        }

        if (this.connectionSettings.MQTT_USE_TLS) {
            mqttClientOptions.protocol = 'mqtts';
        }

        if (this.connectionSettings.MQTT_CERT_FILE) {
            mqttClientOptions.certPath = this.connectionSettings.MQTT_CERT_FILE;
            mqttClientOptions.keyPath = this.connectionSettings.MQTT_KEY_FILE;
            // mqttClientOptions.???? = this.connectionSettings.MQTT_KEY_FILE_PASSWORD;
        }

        if (this.connectionSettings.MQTT_CA_FILE) {
            mqttClientOptions.caPaths = [this.connectionSettings.MQTT_CA_FILE];
        }

        return mqttClientOptions;
    }

    private onConnect(): void {
        Logger.log([ModuleName, 'info'], 'Connected to MQTT broker');
    }

    private onDisconnect(): void {
        Logger.log([ModuleName, 'info'], 'Disconnected from MQTT broker');
    }

    private onMessage(topic: string, payload: Buffer): void {
        Logger.log([ModuleName, 'info'], `Received message on topic ${topic} with payload ${payload.toString('utf8')}`);
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

    private onError(error: Error): void {
        Logger.log([ModuleName, 'error'], `MQTT client error: ${error.message}`);
        Logger.log([ModuleName, 'error'], `This could be authentication error - terminating the sample`);

        process.exit(1);
    }
}
