import {
    IConnackPacket,
    IDisconnectPacket
} from 'mqtt';
import {
    logger,
    GeoJsonPoint,
    MqttConnectionSettings,
    SampleMqttClient,
    TelemetryProducer
} from '@mqttapplicationsamples/mqttjsclientextensions';
import { resolve } from 'path';
import { Command } from 'commander';
import { PositionTelemetryProducer } from './positionTelemetryProducer';

// Parse command line arguments to get the environment file path
const programCommands = new Command();
programCommands
    .requiredOption('-e, --env-file <envFile>', 'Environment filepath')
    .parse(process.argv);
const programOptions = programCommands.opts();

const ModuleName = 'SampleApp';
const VehicleTelemetryPublishIntervalInSeconds = 3;

let sampleApp: SampleApp;

class SampleApp {
    private sampleMqttClient: SampleMqttClient = null as any;

    public async stopSample(): Promise<void> {
        if (this.sampleMqttClient) {
            await this.sampleMqttClient.mqttClient.endAsync(true);

            this.sampleMqttClient = null as any;
        }
    }

    public async startSample(): Promise<void> {
        try {
            logger.info({ tags: [ModuleName] }, `Starting MQTT client telemetry producer`);

            const cs = MqttConnectionSettings.createFromEnvVars(programOptions.envFile);

            // Create the SampleMqttClient instance, this wraps the MQTT.js client
            this.sampleMqttClient = SampleMqttClient.createFromConnectionSettings(cs);

            this.sampleMqttClient.mqttClient.on('connect', this.onConnect.bind(this));
            this.sampleMqttClient.mqttClient.on('disconnect', this.onDisconnect.bind(this));

            // Connect to the MQTT broker using the connection settings from the .env file
            await this.sampleMqttClient.connectAsync();

            const telemetryProducer = new PositionTelemetryProducer(this.sampleMqttClient.mqttClient);

            // Start sending vehicle telemetry data to the 'vehicles/<vehicle-id>/position' topic
            const vehiclePublishTopic = `vehicles/${cs.clientId}/position`;

            while (this.sampleMqttClient) {
                const latMin = -90;
                const latMax = 90;
                const lonMin = -180;
                const lonMax = 180;

                const lat = Math.floor(Math.random() * (latMax - latMin + 1) + latMin);
                const lon = Math.floor(Math.random() * (lonMax - lonMin + 1) + lonMin);

                const pubAck = await telemetryProducer.SendTelemetryAsync(new GeoJsonPoint(lat, lon), 1);

                logger.info({ tags: [ModuleName] }, `Message published on topic '${pubAck.topic}' and mid ${pubAck?.messageId ?? -1}`);

                await new Promise((resolve) => setTimeout(resolve, 1000 * VehicleTelemetryPublishIntervalInSeconds));
            }
        }
        catch (ex) {
            logger.error({ tags: [ModuleName] }, `MQTT client sample error: ${ex.message}`);
        }
    }

    private onConnect(connAck: IConnackPacket): void {
        logger.info({ tags: [ModuleName] }, `Client Connected: ${this.sampleMqttClient.mqttClient.connected} with CONNACK: ${connAck.reasonCode}`);
    }

    private onDisconnect(packet: IDisconnectPacket): void {
        logger.info({ tags: [ModuleName] }, `Mqtt client disconnected with reason: ${packet.reasonCode}`);
    }
}

process.on('SIGINT', async () => {
    logger.error({ tags: [ModuleName] }, `SIGINT received: ending the session and exiting the sample...`);

    if (sampleApp) {
        await sampleApp.stopSample();
    }
});

process.on('SIGTERM', async () => {
    logger.error({ tags: [ModuleName] }, `SIGTERM received: ending the session and exiting the sample...`);

    if (sampleApp) {
        await sampleApp.stopSample();
    }
});

void (async () => {
    sampleApp = new SampleApp();
    await sampleApp.startSample();
})().catch();
