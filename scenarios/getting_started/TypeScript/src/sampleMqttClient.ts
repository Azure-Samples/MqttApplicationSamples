import { Logger } from './logger';
import { IConnectionSettings } from './index';
import {
    ErrorWithReasonCode,
    IClientOptions,
    IConnackPacket,
    IDisconnectPacket,
    IPublishPacket,
    MqttClient,
    connect as mqttConnect
} from 'mqtt';
import { resolve as pathResolve } from 'path';
import * as fs from 'fs';

const ModuleName = 'sampleMqttClient';
const ConnectTimeoutInSeconds = 10;

export class SampleMqttClient {
    private connectionSettings: IConnectionSettings;
    private mqttClient: MqttClient;

    constructor(connectionSettings: IConnectionSettings) {
        this.connectionSettings = connectionSettings;
    }

    public get connected(): boolean {
        return this.mqttClient?.connected || false;
    }

    public async runSample(): Promise<void> {
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
            Logger.log([ModuleName, 'info'], `Starting connection for clientId: ${this.connectionSettings.mqttClientId}`);

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

            Logger.log([ModuleName, 'info'], `MQTT client connected - clientId: ${this.connectionSettings.mqttClientId}`);

            // Subscribe to MQTT topics
            const subscriptionTopics = 'sample/+';

            Logger.log([ModuleName, 'info'], `Subscribing to MQTT topics: ${subscriptionTopics}`);

            await this.mqttClient.subscribeAsync('sample/+');

            // Publish to MQTT topics
            const publishTopic = 'sample/topic1';
            const publishPayload = 'Hello World!';

            Logger.log([ModuleName, 'info'], `Publishing to MQTT topic: ${publishTopic}, with payload: ${publishPayload}`);

            await this.mqttClient.publishAsync(publishTopic, publishPayload);
        }
        catch (ex) {
            Logger.log([ModuleName, 'error'], `MQTT client connect error: ${ex}`);
        }
    }

    private createMqttClientOptions(): IClientOptions {
        let mqttClientOptions: IClientOptions;

        try {
            mqttClientOptions = {
                clientId: this.connectionSettings.mqttClientId,
                username: this.connectionSettings.mqttClientId,
                protocol: 'mqtt',
                host: this.connectionSettings.mqttHostname,
                port: this.connectionSettings.mqttTcpPort,
                keepalive: this.connectionSettings.mqttKeepAliveInSeconds,
                connectTimeout: ConnectTimeoutInSeconds * 1000,
                rejectUnauthorized: true,
                manualConnect: true,
                clean: this.connectionSettings.mqttCleanSession,
                protocolVersion: 5
            };

            if (this.connectionSettings.mqttUsername) {
                mqttClientOptions.username = this.connectionSettings.mqttUsername;
                mqttClientOptions.password = this.connectionSettings.mqttPassword;
            }

            if (this.connectionSettings.mqttUseTls) {
                mqttClientOptions.protocol = 'mqtts';
            }

            if (this.connectionSettings.mqttCertFile) {
                mqttClientOptions.cert = fs.readFileSync(pathResolve('..', this.connectionSettings.mqttCertFile));
                mqttClientOptions.key = fs.readFileSync(pathResolve('..', this.connectionSettings.mqttKeyFile));
            }

            if (this.connectionSettings.mqttCaFile) {
                mqttClientOptions.ca = fs.readFileSync(pathResolve('..', this.connectionSettings.mqttCaFile));
            }
        }
        catch (ex) {
            Logger.log([ModuleName, 'error'], `Error while creating client options: ${ex}`);
        }

        return mqttClientOptions;
    }

    private onConnect(_packet: IConnackPacket): void {
        Logger.log([ModuleName, 'info'], `Connected to MQTT broker: ${this.connectionSettings.mqttHostname}:${this.connectionSettings.mqttTcpPort}`);
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

        Logger.log([ModuleName, 'error'], `Terminating the Getting Started sample...`);

        process.exit(1);
    }
}
