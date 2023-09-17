import { Logger } from './logger';
import { ConnectionSettings } from './connectionSettings';
import {
    ErrorWithReasonCode,
    IClientOptions,
    IConnackPacket,
    IDisconnectPacket,
    IPublishPacket,
    MqttClient,
    connect as mqttConnect
} from 'mqtt';
import * as fs from 'fs';

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
        try {
            Logger.log([ModuleName, 'info'], `Initializing MQTT client`);
            const mqttClientOptions: IClientOptions = this.createMqttClientOptions();

            this.mqttClient = mqttConnect(mqttClientOptions);

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
        catch (ex) {
            Logger.log([ModuleName, 'error'], `MQTT client Getting Started error: ${ex}`);
        }
    }

    private createMqttClientOptions(): IClientOptions {
        if (!this.connectionSettings.MQTT_CLEAN_SESSION) {
            throw new Error('This sample does not support connecting with existing sessions');
        }

        if (!this.connectionSettings.MQTT_HOST_NAME) {
            throw new Error('MQTT_HOST_NAME environment variable is not set');
        }

        if (this.connectionSettings.MQTT_PASSWORD && !this.connectionSettings.MQTT_USERNAME) {
            throw new Error('MQTT_USERNAME environment variable if MQTT_PASSWORD is set');
        }

        const mqttClientOptions: IClientOptions = {
            clientId: this.connectionSettings.MQTT_CLIENT_ID,
            protocol: 'mqtt',
            host: this.connectionSettings.MQTT_HOST_NAME,
            port: this.connectionSettings.MQTT_TCP_PORT,
            keepalive: this.connectionSettings.MQTT_KEEP_ALIVE_IN_SECONDS,
            connectTimeout: 10 * 1000,
            rejectUnauthorized: true,
            manualConnect: true,
            clean: this.connectionSettings.MQTT_CLEAN_SESSION,
            protocolVersion: 5
        };

        if (this.connectionSettings.MQTT_USERNAME) {
            mqttClientOptions.username = this.connectionSettings.MQTT_USERNAME;
            mqttClientOptions.password = this.connectionSettings.MQTT_PASSWORD;
        }

        if (this.connectionSettings.MQTT_USE_TLS) {
            mqttClientOptions.protocol = 'mqtts';
        }

        if (this.connectionSettings.MQTT_CERT_FILE) {
            mqttClientOptions.cert = fs.readFileSync(this.connectionSettings.MQTT_CERT_FILE);
            mqttClientOptions.key = fs.readFileSync(this.connectionSettings.MQTT_KEY_FILE);
        }

        if (this.connectionSettings.MQTT_CA_FILE) {
            mqttClientOptions.ca = fs.readFileSync(this.connectionSettings.MQTT_CA_FILE);
        }

        return mqttClientOptions;
    }

    private onConnect(_packet: IConnackPacket): void {
        Logger.log([ModuleName, 'info'], 'Connected to MQTT broker');
    }

    private onDisconnect(_packet: IDisconnectPacket): void {
        Logger.log([ModuleName, 'info'], 'Disconnected from MQTT broker');
    }

    private onMessage(topic: string, payload: Buffer, _packet: IPublishPacket): void {
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
