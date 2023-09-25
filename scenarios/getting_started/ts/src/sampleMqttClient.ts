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

const ModuleName = 'SampleMqttClient';
const ConnectTimeoutInSeconds = 10;

export class SampleMqttClient {
    private mqttClient: MqttClient;

    public get connected(): boolean {
        return this.mqttClient?.connected || false;
    }

    public async endClientSession(): Promise<void> {
        if (this.mqttClient?.connected) {
            await this.mqttClient.endAsync(true);
        }
    }

    public createMqttClientOptions(connectionSettings: IConnectionSettings): IClientOptions {
        let mqttClientOptions: IClientOptions;

        try {
            mqttClientOptions = {
                clientId: connectionSettings.mqttClientId,
                protocol: 'mqtt',
                host: connectionSettings.mqttHostname,
                port: connectionSettings.mqttTcpPort,
                keepalive: connectionSettings.mqttKeepAliveInSeconds,
                connectTimeout: ConnectTimeoutInSeconds * 1000,
                rejectUnauthorized: true,
                manualConnect: true,
                clean: connectionSettings.mqttCleanSession,
                protocolVersion: 5
            };

            if (connectionSettings.mqttUsername) {
                mqttClientOptions.username = connectionSettings.mqttUsername;
                mqttClientOptions.password = connectionSettings.mqttPassword;
            }

            if (connectionSettings.mqttUseTls) {
                mqttClientOptions.protocol = 'mqtts';
            }

            if (connectionSettings.mqttCertFile) {
                mqttClientOptions.cert = fs.readFileSync(pathResolve('..', connectionSettings.mqttCertFile));
                mqttClientOptions.key = fs.readFileSync(pathResolve('..', connectionSettings.mqttKeyFile));
            }

            if (connectionSettings.mqttCaFile) {
                mqttClientOptions.ca = fs.readFileSync(pathResolve('..', connectionSettings.mqttCaFile));
            }
        }
        catch (ex) {
            Logger.log([ModuleName, 'error'], `Error while creating client options: ${ex}`);
        }

        return mqttClientOptions;
    }

    public async connect(connectionSettings: IConnectionSettings): Promise<void> {
        try {
            Logger.log([ModuleName, 'info'], `Initializing MQTT client`);

            const mqttClientOptions: IClientOptions = this.createMqttClientOptions(connectionSettings);

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
            Logger.log([ModuleName, 'info'], `Starting connection for clientId: ${this.mqttClient.options.clientId}`);

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

            Logger.log([ModuleName, 'info'], `MQTT client connected - clientId: ${this.mqttClient.options.clientId}`);
        }
        catch (ex) {
            Logger.log([ModuleName, 'error'], `MQTT client connect error: ${ex}`);
        }
    }

    public async subscribe(topic: string): Promise<void> {
        try {
            Logger.log([ModuleName, 'info'], `Subscribing to MQTT topics: ${topic}`);

            await this.mqttClient.subscribeAsync(topic);
        }
        catch (ex) {
            Logger.log([ModuleName, 'error'], `MQTT client subscribe error: ${ex}`);
        }
    }

    public async publish(topic: string, payload: string): Promise<void> {
        try {
            Logger.log([ModuleName, 'info'], `Publishing to MQTT topic: ${topic}, with payload: ${payload}`);

            await this.mqttClient.publishAsync(topic, payload);
        }
        catch (ex) {
            Logger.log([ModuleName, 'error'], `MQTT client publish error: ${ex}`);
        }
    }

    private onConnect(_packet: IConnackPacket): void {
        Logger.log([ModuleName, 'info'], `Connected to MQTT broker: ${this.mqttClient.options.host}:${this.mqttClient.options.port}`);
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
