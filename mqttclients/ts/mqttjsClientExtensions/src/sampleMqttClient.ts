import { logger } from './logger';
import { MqttConnectionSettings } from './mqttConnectionSettings';
import {
    ErrorWithReasonCode,
    MqttClient,
    connect as mqttConnect
} from 'mqtt';

const ModuleName = 'SampleMqttClient';

export class SampleMqttClient {
    public static createFromConnectionSettings(cs: MqttConnectionSettings): SampleMqttClient {
        let mqttSampleClient: SampleMqttClient = null as any;

        try {
            logger.info({ tags: [ModuleName] }, `Creating instance of SampleMqttClient for clientId: ${cs.clientId}`);

            const mqttClient = mqttConnect(MqttConnectionSettings.createMqttClientOptions(cs));

            mqttSampleClient = new SampleMqttClient(mqttClient);
        }
        catch (ex) {
            logger.error({ tags: [ModuleName] }, `Error while creating instance of SampleMqttClient: ${ex.message}`);
        }

        return mqttSampleClient;
    }

    public mqttClient: MqttClient;

    constructor(mqttClient: MqttClient) {
        this.mqttClient = mqttClient;
    }

    public async connectAsync(): Promise<void> {
        try {
            this.mqttClient.on('close', this.onClose.bind(this));
            this.mqttClient.on('end', this.onEnd.bind(this));
            this.mqttClient.on('reconnect', this.onReconnect.bind(this));
            this.mqttClient.on('offline', this.onOffline.bind(this));
            this.mqttClient.on('error', this.onError.bind(this));

            // Connect to MQTT broker
            logger.info({ tags: [ModuleName] }, `Starting connection for clientId: ${this.mqttClient.options.clientId}`);

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
                await this.mqttClient.endAsync(true);

                throw new Error('Unable to connect to MQTT broker');
            }

            logger.info({ tags: [ModuleName] }, `MQTT client connected - clientId: ${this.mqttClient.options.clientId}`);
        }
        catch (ex) {
            logger.error({ tags: [ModuleName] }, `MQTT client connect error: ${ex.message}`);
        }
    }

    private onClose(): void {
        logger.info({ tags: [ModuleName] }, 'MQTT broker connection closed');
    }

    private onEnd(): void {
        logger.info({ tags: [ModuleName] }, 'MQTT broker connection ended');
    }

    private onReconnect(): void {
        logger.info({ tags: [ModuleName] }, 'MQTT broker session re-connected');
    }

    private onOffline(): void {
        logger.info({ tags: [ModuleName] }, 'MQTT broker connection is offline');
    }

    private onError(error: Error | ErrorWithReasonCode): void {
        logger.error({ tags: [ModuleName] }, `MQTT client error:`);

        if ((error as ErrorWithReasonCode)?.code) {
            logger.error({ tags: [ModuleName] }, `  - reason code: ${(error as ErrorWithReasonCode).code}`);

            if ((error as ErrorWithReasonCode)?.message) {
                logger.error({ tags: [ModuleName] }, `  - message: ${(error as ErrorWithReasonCode).message}`);
            }
            else {
                const errors = (error as any).errors;
                if (errors && Array.isArray(errors)) {
                    for (const subError of errors) {
                        logger.error({ tags: [ModuleName] }, `  - message: ${subError.message}`);
                    }
                }
            }
        }

        logger.error({ tags: [ModuleName] }, `Terminating the sample...`);

        process.exit(1);
    }
}
