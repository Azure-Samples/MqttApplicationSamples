import {
    IConnackPacket,
    IPublishPacket
} from 'mqtt';

import {
    logger,
    MqttConnectionSettings,
    SampleMqttClient
} from '@mqttapplicationsamples/mqttjsclientextensions';
import { resolve } from 'path';

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
            logger.info({ tags: [ModuleName] }, `Starting MQTT client sample`);

            const cs = MqttConnectionSettings.createFromEnvVars(resolve(__dirname, '../../.env'));

            // Create the SampleMqttClient instance, this wraps the MQTT.js client
            this.sampleMqttClient = SampleMqttClient.createFromConnectionSettings(cs);

            this.sampleMqttClient.mqttClient.on('connect', this.onConnect.bind(this));
            this.sampleMqttClient.mqttClient.on('message', this.onMessage.bind(this));

            // Connect to the MQTT broker using the connection settings from the .env file
            await this.sampleMqttClient.connectAsync();

            const subscribeTopic = 'sample/+';

            logger.info({ tags: [ModuleName] }, `Subscribing to MQTT topic: ${subscribeTopic}`);

            await this.sampleMqttClient.mqttClient.subscribeAsync(subscribeTopic, {
                qos: 1
            });

            const publishTopic = 'sample/topic1';

            logger.info({ tags: [ModuleName] }, `Publishing to MQTT topic: ${publishTopic}`);

            await this.sampleMqttClient.mqttClient.publishAsync('sample/topic1', 'Hello World!');
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
