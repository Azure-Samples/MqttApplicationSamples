import {
    IConnackPacket,
    IDisconnectPacket
} from 'mqtt';
import { Command } from 'commander';
import {
    logger,
    MqttConnectionSettings,
    SampleMqttClient
} from '@mqttapplicationsamples/mqttjsclientextensions';
import { UnlockRequest } from '@mqttapplicationsamples/protomessages';
import { UnlockCommandClient } from './unlockCommandClient';

// Parse command line arguments to get the environment file path
const programCommands = new Command();
programCommands
    .requiredOption('-e, --env-file <envFile>', 'Environment filepath')
    .parse(process.argv);

// Load environment variables from .env file using the dotenv package
const programOptions = programCommands.opts();

const ModuleName = 'SampleApp';

let sampleApp: SampleApp;

class SampleApp {
    private sampleMqttClient: SampleMqttClient;

    public async stopSample(): Promise<void> {
        if (this.sampleMqttClient) {
            await this.sampleMqttClient.mqttClient.endAsync(true);
        }
    }

    public async startSample(): Promise<void> {
        try {
            logger.info({ tags: [ModuleName] }, `Starting MQTT command client`);

            const cs = MqttConnectionSettings.createFromEnvVars(programOptions.envFile);

            // Create the SampleMqttClient instance, this wraps the MQTT.js client
            this.sampleMqttClient = SampleMqttClient.createFromConnectionSettings(cs);

            this.sampleMqttClient.mqttClient.on('connect', this.onConnect.bind(this));
            this.sampleMqttClient.mqttClient.on('disconnect', this.onDisconnect.bind(this));

            // Connect to the MQTT broker using the connection settings from the .env file
            await this.sampleMqttClient.connectAsync();

            const commandClient = new UnlockCommandClient(this.sampleMqttClient.mqttClient);

            try {
                logger.info({ tags: [ModuleName] }, `Invoking unlock command: ${new Date().toISOString()}`);

                const unlockRequest = UnlockRequest.create({
                    when: new Date(),
                    requestedFrom: this.sampleMqttClient.mqttClient.options.clientId
                });

                const response = await commandClient.invokeAsync("vehicle03", unlockRequest, 30000);

                logger.info({ tags: [ModuleName] }, `Command response succeed=${response.succeed}`);
            }
            catch (ex) {
                logger.error({ tags: [ModuleName] }, `Command response error: ${ex.message}`);
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
