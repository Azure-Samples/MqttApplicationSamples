import { Logger } from './logger';
import { SampleMqttClient } from './sampleMqttClient';
import { resolve } from 'path';
import { Command } from 'commander';
import { config } from 'dotenv';

// Parse command line arguments to get the environment file path
const programCommands = new Command();
programCommands
    .requiredOption('-e, --env-file <envFile>', 'Environment filepath')
    .parse(process.argv);

// Load environment variables from .env file using the dotenv package
const programOptions = programCommands.opts();
const envConfig = config({ path: resolve(__dirname, `../../${programOptions.envFile}`) });

const ModuleName = 'SampleApp';
const VehicleTelemetryPublishIntervalInSeconds = 3;

let sampleApp: SampleApp;

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

class GeoJsonPoint {
    constructor(x: number, y: number) {
        this.coordinates[0] = x;
        this.coordinates[1] = y;
    }

    public type: string = 'Point';
    public coordinates: number[] = [0, 0];
}

class SampleApp {
    private sampleMqttClient: SampleMqttClient;
    private vehicleTelemetryPublishIntervalId: NodeJS.Timeout;

    public async stopSample(): Promise<void> {
        if (this.vehicleTelemetryPublishIntervalId) {
            clearInterval(this.vehicleTelemetryPublishIntervalId);
            this.vehicleTelemetryPublishIntervalId = null;
        }

        if (this.sampleMqttClient?.connected) {
            await this.sampleMqttClient.endClientSession();
        }
    }

    public async startSample(): Promise<void> {
        try {
            Logger.log([ModuleName, 'info'], `Starting MQTT client sample`);

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

            // If the environment file is 'map-app.env', we treat this app instance
            // as the "consumer" of the vehicle telemetry data and subscribe to the
            // 'vehicles/+/position' topic
            //
            // Otherwise if the environment file is any other (e.g. vehicle01.env),
            // we treat this app instance as the "producer" of the vehicle telemetry
            // data and publish to the 'vehicles/<vehicle-id>/position' topic

            if (programOptions.envFile === 'map-app.env') {
                // Subscribe to the vehicle telemetry data topic
                await this.sampleMqttClient.subscribe('vehicles/+/position');
            }
            else {
                // Start sending vehicle telemetry data to the 'vehicles/<vehicle-id>/position' topic
                const vehiclePublishTopic = `vehicles/${connectionSettings.mqttClientId}/position`;

                this.vehicleTelemetryPublishIntervalId = setInterval(async () => {
                    const latMin = -90;
                    const latMax = 90;
                    const lonMin = -180;
                    const lonMax = 180;

                    const lat = Math.floor(Math.random() * (latMax - latMin + 1) + latMin);
                    const lon = Math.floor(Math.random() * (lonMax - lonMin + 1) + lonMin);
                    const vehiclePosition = new GeoJsonPoint(lat, lon);
                    const payload = JSON.stringify(vehiclePosition);

                    const messageId = await this.sampleMqttClient.publish(vehiclePublishTopic, payload);

                    Logger.log([ModuleName, 'info'], `Publishing vehicle geolocation data - messageId: ${messageId}, topic ${vehiclePublishTopic}, payload: ${payload}`);
                }, 1000 * VehicleTelemetryPublishIntervalInSeconds);
            }
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
