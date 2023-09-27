import {
    Logger,
    ConnectionSettings,
    SampleMqttClient
} from '@mqttapplicationsamples/mqttjsclientextensions';
import { resolve } from 'path';
import { Command } from 'commander';

// Parse command line arguments to get the environment file path
const programCommands = new Command();
programCommands
    .requiredOption('-e, --env-file <envFile>', 'Environment filepath')
    .parse(process.argv);

// Load environment variables from .env file using the dotenv package
const programOptions = programCommands.opts();

const ModuleName = 'SampleApp';
const VehicleTelemetryPublishIntervalInSeconds = 3;

let sampleApp: SampleApp;

class GeoJsonPoint {
    constructor(x: number, y: number) {
        this.coordinates[0] = x;
        this.coordinates[1] = y;
    }

    public type = 'Point';
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

            const cs = ConnectionSettings.createFromEnvVars(resolve(__dirname, '../../.env'));

            // Create the SampleMqttClient instance, this wraps the MQTT.js client
            this.sampleMqttClient = new SampleMqttClient();

            // Connect to the MQTT broker using the connection settings from the .env file
            await this.sampleMqttClient.connect(cs);

            const commandRequestTopic = `vehicles/${cs.clientId}/command/unlock/request`;
            // const commandResonseTopic = `vehicles/${connectionSettings.mqttClientId}/command/unlock/response`;

            // If the environment file is 'map-app.env', we treat this app instance
            // as the "client" sending the vehicle unlock command by publishing to the
            // 'vehicles/<vehicle-id>/command/unlock/request' topic and subscribing to
            // the 'vehicles/<vehicle-id>/command/unlock/response' topic.
            //
            // Otherwise if the environment file is any other (e.g. vehicle03.env),
            // we treat this app instance as the "server" receiving the vehicle unlock
            // command by subscribing to the 'vehicles/<vehicle-id>/position' topic and
            // publishing to the 'vehicles/<vehicle-id>/command/unlock/response' topic.

            if (programOptions.envFile === 'map-app.env') {
                // Send unlock command to the vehicle
                await this.sampleMqttClient.publish(commandRequestTopic, 'unlock');

                await this.sampleMqttClient.subscribe('vehicles/+/position');
            }
            else {
                // Start sending vehicle telemetry data to the 'vehicles/<vehicle-id>/position' topic
                const vehiclePublishTopic = `vehicles/${cs.clientId}/position`;

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
