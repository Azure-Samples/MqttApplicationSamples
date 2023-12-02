import {
    IConnackPacket,
    IDisconnectPacket,
    IPublishPacket
} from 'mqtt';
import {
    logger,
    MqttConnectionSettings,
    SampleMqttClient
} from '@mqttapplicationsamples/mqttjsclientextensions';
import { resolve } from 'path';
import { Command } from 'commander';

// Parse command line arguments to get the environment file path
const programCommands = new Command();
programCommands
    .requiredOption('-e, --env-file <envFile>', 'Environment filepath')
    .parse(process.argv);
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
            this.vehicleTelemetryPublishIntervalId = null as any;
        }

        if (this.sampleMqttClient) {
            await this.sampleMqttClient.mqttClient.endAsync(true);
        }
    }

    public async startSample(): Promise<void> {
        try {
            logger.info({ tags: [ModuleName] }, `Starting MQTT client telemetry consumer`);

            const cs = MqttConnectionSettings.createFromEnvVars(resolve(__dirname, `../../../${programOptions.envFile}`));

            // Create the SampleMqttClient instance, this wraps the MQTT.js client
            this.sampleMqttClient = SampleMqttClient.createFromConnectionSettings(cs);

            this.sampleMqttClient.mqttClient.on('connect', this.onConnect.bind(this));
            this.sampleMqttClient.mqttClient.on('message', this.onMessage.bind(this));

            // Connect to the MQTT broker using the connection settings from the .env file
            await this.sampleMqttClient.connectAsync();

            // If the environment file is 'map-app.env', we treat this app instance
            // as the "consumer" of the vehicle telemetry data and subscribe to the
            // 'vehicles/+/position' topic
            //
            // Otherwise if the environment file is any other (e.g. vehicle01.env),
            // we treat this app instance as the "producer" of the vehicle telemetry
            // data and publish to the 'vehicles/<vehicle-id>/position' topic

            if (programOptions.envFile === 'map-app.env') {
                const topic = 'vehicles/+/position';

                logger.info({ tags: [ModuleName] }, `Subscribing to MQTT topics: ${topic}`);

                await this.sampleMqttClient.mqttClient.subscribeAsync(topic, {
                    qos: 1
                });
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

                    const publishPacket = await this.sampleMqttClient.mqttClient.publishAsync(vehiclePublishTopic, payload, {
                        qos: 1
                    });

                    logger.info({ tags: [ModuleName] }, `Publishing vehicle geolocation data - messageId: ${publishPacket?.messageId ?? -1}, topic ${vehiclePublishTopic}, payload: ${payload}`);
                }, 1000 * VehicleTelemetryPublishIntervalInSeconds);
            }
        }
        catch (ex) {
            logger.error({ tags: [ModuleName] }, `MQTT client sample error: ${ex.message}`);
        }
    }

    private onConnect(connAck: IConnackPacket): void {
        logger.info({ tags: [ModuleName] }, `Client Connected: ${this.sampleMqttClient.mqttClient.connected} with CONNACK: ${connAck.reasonCode}`);
    }

    private onMessage(topic: string, payload: Buffer, _packet: IPublishPacket): void {
        logger.info({ tags: [ModuleName] }, `Received message on topic: '${topic}' with content: '${payload.toString('utf8')}'`);
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
