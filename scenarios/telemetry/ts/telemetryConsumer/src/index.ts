import {
    IConnackPacket,
    IDisconnectPacket
} from 'mqtt';
import {
    logger,
    GeoJsonPoint,
    MqttConnectionSettings,
    SampleMqttClient,
    TelemetryMessage,
    TelemetryConsumer
} from '@mqttapplicationsamples/mqttjsclientextensions';
import { Command } from 'commander';
import { PositionTelemetryConsumer } from './positionTelemetryConsumer';

// Parse command line arguments to get the environment file path
const programCommands = new Command();
programCommands
    .requiredOption('-e, --env-file <envFile>', 'Environment filepath')
    .parse(process.argv);
const programOptions = programCommands.opts();

const ModuleName = 'TelemetryConsumerApp';
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
            logger.info({ tags: [ModuleName] }, `Starting MQTT client telemetry consumer`);

            const cs = MqttConnectionSettings.createFromEnvVars(programOptions.envFile);

            // Create the SampleMqttClient instance, this wraps the MQTT.js client
            this.sampleMqttClient = SampleMqttClient.createFromConnectionSettings(cs);

            this.sampleMqttClient.mqttClient.on('connect', this.onConnect.bind(this));
            this.sampleMqttClient.mqttClient.on('disconnect', this.onDisconnect.bind(this));

            // Connect to the MQTT broker using the connection settings from the .env file
            await this.sampleMqttClient.connectAsync();

            const telemetryConsumer = new PositionTelemetryConsumer(this.sampleMqttClient.mqttClient);
            telemetryConsumer.onTelemetryReceived = async (msg: TelemetryMessage<GeoJsonPoint>): Promise<void> => {
                await new Promise((resolve, reject) => {
                    process.nextTick(resolve, reject);
                });

                logger.info({ tags: [ModuleName] }, `Received msg from ${msg.clientIdFromTopic}. Coordinates lat: ${msg.payload.coordinates[0]}, lon: ${msg.payload.coordinates[1]}`);
            };

            await telemetryConsumer.startAsync();
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
