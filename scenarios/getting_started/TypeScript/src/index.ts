import { Logger } from './logger';
import { SampleMqttClient } from './sampleMqttClient';
import { resolve } from 'path';
import { config } from 'dotenv';

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

const ModuleName = 'index';

async function start(): Promise<void> {
    try {
        Logger.log([ModuleName, 'info'], `Starting MQTT client Getting Started sample}`);

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

        const sampleMqttClient = new SampleMqttClient(connectionSettings);

        await sampleMqttClient.runSample();
    }
    catch (ex) {
        Logger.log([ModuleName, 'error'], `MQTT client Getting Started error: ${ex.message}`);
    }
}

void (async () => {
    await start();
})().catch();
