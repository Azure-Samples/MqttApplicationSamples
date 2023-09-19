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

export interface Point {
    x: number;
    y: number;
}

const ModuleName = 'index';

let sampleMqttClient: SampleMqttClient = null;

process.on('SIGINT', async () => {
    if (sampleMqttClient?.connected) {
        await sampleMqttClient.stopSample();
    }
});

process.on('SIGTERM', async () => {
    if (sampleMqttClient?.connected) {
        await sampleMqttClient.stopSample();
    }
});

async function start(): Promise<void> {
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
        sampleMqttClient = new SampleMqttClient();

        // Connect to the MQTT broker using the connection settings from the .env file
        await sampleMqttClient.connect(connectionSettings);

        // Publish to the 'sample/topic1' topic
        const topic = `vehicles/${connectionSettings.mqttClientId}/position`;

        await sampleMqttClient.publish('sample/topic1', 'Hello World!');

        // Begin sending vehicle geolocation data to the 'vehicles/<vehicleId>/position' topic
        const intervalId = setInterval(async () => {
            const latMin = -90;
            const latMax = 90;
            const lonMin = -180;
            const lonMax = 180;

            const point: Point = {
                x: Math.floor(Math.random() * (latMax - latMin + 1) + latMin),
                y: Math.floor(Math.random() * (lonMax - lonMin + 1) + lonMin)
            };

            const messageId = await sampleMqttClient.publish(topic, JSON.stringify(point));

            Logger.log([ModuleName, 'info'], `Publishing to topic: ${topic}, messageId: ${messageId}, with payload: ${JSON.stringify(point, null, 4)}`);
        }, 1000 * 10);
    }
    catch (ex) {
        Logger.log([ModuleName, 'error'], `MQTT client sample error: ${ex.message}`);
    }
}

void (async () => {
    await start();
})().catch();
