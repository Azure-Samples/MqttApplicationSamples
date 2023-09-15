import { Logger } from './logger';
import { ConnectionSettings } from './connectionSettings';
import { SampleMqttClient } from './sampleMqttClient';

const ModuleName = 'index';
const connectionSettings = ConnectionSettings.instance;

async function start(): Promise<void> {
    try {
        Logger.log([ModuleName, 'info'], `Starting MQTT client Getting Started sample`);

        const sampleMqttClient = new SampleMqttClient(connectionSettings);

        await sampleMqttClient.connect();
    }
    catch (ex) {
        Logger.log([ModuleName, 'error'], `MQTT client Getting Started error: ${ex}`);
    }
}

void (async () => {
    await start();
})().catch();
