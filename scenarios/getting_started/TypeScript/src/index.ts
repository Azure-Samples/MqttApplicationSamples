import { Logger } from './logger';
import { SampleMqttClient } from './sampleMqttClient';
import { resolve } from 'path';
import { config } from 'dotenv';

// Load environment variables from .env file using the dotenv package
const envConfig = config({ path: resolve(__dirname, '../../.env') });

export interface IConnectionSettings {
    mqttHostname: string;
    mqttTcpPort: number;
    mqttUseTls: boolean;
    mqttCleanSession: boolean;
    mqttKeepAliveInSeconds: number;
    mqttClientId: string;
    mqttUsername: string;
    mqttPassword: string;
    mqttCertFile: string;
    mqttKeyFile: string;
    mqttCaFile: string;
}

const ModuleName = 'SampleApp';

let sampleApp: SampleApp;

class SampleApp {
    private sampleMqttClient: SampleMqttClient;

    public async stopSample(): Promise<void> {
        if (this.sampleMqttClient?.connected) {
            await this.sampleMqttClient.endClientSession();
        }
    }

    public async startSample(): Promise<void> {
        try {
            Logger.log([ModuleName, 'info'], `Starting MQTT client sample}`);

            if (!envConfig.parsed?.MQTT_HOST_NAME) {
                throw new Error('MQTT_HOST_NAME environment variable is not set');
            }

            if (envConfig.parsed?.MQTT_PASSWORD && !envConfig.parsed?.MQTT_USERNAME) {
                throw new Error('MQTT_USERNAME environment variable is required if MQTT_PASSWORD is set');
            }

            const connectionSettings: IConnectionSettings = {
                mqttHostname: envConfig.parsed?.MQTT_HOST_NAME || '',
                mqttTcpPort: Number(envConfig.parsed?.MQTT_TCP_PORT) || 8883,
                mqttUseTls: Boolean(envConfig.parsed?.MQTT_USE_TLS === undefined ? true : envConfig.parsed?.MQTT_USE_TLS),
                mqttCleanSession: Boolean(envConfig.parsed?.MQTT_CLEAN_SESSION === undefined ? true : envConfig.parsed?.MQTT_CLEAN_SESSION),
                mqttKeepAliveInSeconds: Number(envConfig.parsed?.MQTT_KEEP_ALIVE_IN_SECONDS) || 30,
                mqttClientId: envConfig.parsed?.MQTT_CLIENT_ID || '',
                mqttUsername: envConfig.parsed?.MQTT_USERNAME || '',
                mqttPassword: envConfig.parsed?.MQTT_PASSWORD || '',
                mqttCertFile: envConfig.parsed?.MQTT_CERT_FILE || '',
                mqttKeyFile: envConfig.parsed?.MQTT_KEY_FILE || '',
                mqttCaFile: envConfig.parsed?.MQTT_CA_FILE || ''
            };

            // Create the SampleMqttClient instance, this wraps the MQTT.js client
            this.sampleMqttClient = new SampleMqttClient();

            // Connect to the MQTT broker using the connection settings from the .env file
            await this.sampleMqttClient.connect(connectionSettings);

            // Subscribe to the 'sample/+' topic
            await this.sampleMqttClient.subscribe('sample/+');

            // Publish to the 'sample/topic1' topic
            await this.sampleMqttClient.publish('sample/topic1', 'Hello World!');
        }
        catch (ex) {
            Logger.log([ModuleName, 'error'], `MQTT client sample error: ${ex.message}`);
        }
    }
}

process.on('SIGINT', async () => {
    Logger.log([ModuleName, 'error'], `SIGINT received: ending the session and exiting the sample...`);

    if (sampleApp) {
        await sampleApp.stopSample();
    }
});

process.on('SIGTERM', async () => {
    Logger.log([ModuleName, 'error'], `SIGTERM received: ending the session and exiting the sample...`);

    if (sampleApp) {
        await sampleApp.stopSample();
    }
});

void (async () => {
    sampleApp = new SampleApp();
    await sampleApp.startSample();
})().catch();
