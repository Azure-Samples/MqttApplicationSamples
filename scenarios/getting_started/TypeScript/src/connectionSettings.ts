import { resolve } from 'path';
import { config } from 'dotenv';

config({ path: resolve(__dirname, '../../.env') });

export class ConnectionSettings {
    private static _instance: ConnectionSettings;

    private constructor() {
        // ...
    }

    public static get instance(): ConnectionSettings {
        /* eslint-disable no-underscore-dangle */
        if (!ConnectionSettings._instance) {
            ConnectionSettings._instance = new ConnectionSettings();
        }

        return ConnectionSettings._instance;
        /* eslint-disable no-underscore-dangle */
    }

    public get MQTT_HOST_NAME(): string {
        return process.env.MQTT_HOST_NAME || '';
    }

    public get MQTT_TCP_PORT(): number {
        return Number(process.env.MQTT_TCP_PORT) || 8883;
    }

    public get MQTT_USE_TLS(): boolean {
        return Boolean(process.env.MQTT_USE_TLS === undefined ? true : process.env.MQTT_USE_TLS);
    }

    public get MQTT_CLEAN_SESSION(): boolean {
        return Boolean(process.env.MQTT_CLEAN_SESSION === undefined ? true : process.env.MQTT_CLEAN_SESSION);
    }

    public get MQTT_KEEP_ALIVE_IN_SECONDS(): number {
        return Number(process.env.MQTT_KEEP_ALIVE_IN_SECONDS) || 30;
    }

    public get MQTT_CLIENT_ID(): string {
        return process.env.MQTT_CLIENT_ID || '';
    }

    public get MQTT_USERNAME(): string {
        return process.env.MQTT_USERNAME || '';
    }

    public get MQTT_PASSWORD(): string {
        return process.env.MQTT_PASSWORD || '';
    }

    public get MQTT_CA_FILE(): string {
        return process.env.MQTT_CA_FILE || '';
    }

    public get MQTT_CERT_FILE(): string {
        return process.env.MQTT_CERT_FILE || '';
    }

    public get MQTT_KEY_FILE(): string {
        return process.env.MQTT_KEY_FILE || '';
    }

    public get MQTT_KEY_FILE_PASSWORD(): string {
        return process.env.MQTT_KEY_FILE_PASSWORD || '';
    }
}
