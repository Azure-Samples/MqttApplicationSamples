import { config } from 'dotenv';

enum AuthType {
    X509 = 'X509',
    Basic = 'Basic'
}

const defaultHostname = 'localhost';
const defaultKeepAliveInSeconds = 30;
const defaultCleanSession = true;
const defaultTcpPort = 8883;
const defaultUseTls = true;
const defaultDisableCrl = false;

export class MqttConnectionSettings {
    private _hostname = defaultHostname;
    public clientId = '';
    public certFile = '';
    public keyFile = '';
    public keyFilePassword = '';
    public username = '';
    public password = '';
    public keepAliveInSeconds: number = defaultKeepAliveInSeconds;
    public cleanSession: boolean = defaultCleanSession;
    public tcpPort: number = defaultTcpPort;
    public useTls: boolean = defaultUseTls;
    public caFile = '';
    public disableCrl: boolean = defaultDisableCrl;

    constructor(hostname: string) {
        this._hostname = hostname;
    }

    public static fromConnectionString(connectionString: string): MqttConnectionSettings {
        return MqttConnectionSettings.parseConnectionString(connectionString);
    }

    public static createFromEnvVars(envFilePath = '.env'): MqttConnectionSettings {
        // Load environment variables from .env file using the dotenv package
        const envConfig = config({ path: envFilePath });

        if (envConfig.error) {
            throw new Error(envConfig.error.message);
        }

        const hostname = envConfig.parsed?.MQTT_HOST_NAME;

        if (!hostname) {
            throw new Error('MQTT_HOST_NAME environment variable is not set');
        }

        if (envConfig.parsed?.MQTT_PASSWORD && !envConfig.parsed?.MQTT_USERNAME) {
            throw new Error('MQTT_USERNAME environment variable is required if MQTT_PASSWORD is set');
        }

        const cs = new MqttConnectionSettings(hostname);
        cs.tcpPort = Number(envConfig.parsed?.MQTT_TCP_PORT ?? defaultTcpPort);
        cs.useTls = Boolean(envConfig.parsed?.MQTT_USE_TLS === undefined ? defaultUseTls : envConfig.parsed?.MQTT_USE_TLS);
        cs.cleanSession = Boolean(envConfig.parsed?.MQTT_CLEAN_SESSION === undefined ? defaultCleanSession : envConfig.parsed?.MQTT_CLEAN_SESSION);
        cs.keepAliveInSeconds = Number(envConfig.parsed?.MQTT_KEEP_ALIVE_IN_SECONDS ?? defaultKeepAliveInSeconds);
        cs.clientId = envConfig.parsed?.MQTT_CLIENT_ID ?? '';
        cs.username = envConfig.parsed?.MQTT_USERNAME ?? '';
        cs.password = envConfig.parsed?.MQTT_PASSWORD ?? '';
        cs.certFile = envConfig.parsed?.MQTT_CERT_FILE ?? '';
        cs.keyFile = envConfig.parsed?.MQTT_KEY_FILE ?? '';
        cs.caFile = envConfig.parsed?.MQTT_CA_FILE ?? '';

        return cs;
    }

    public static parseConnectionString(connectionString: string): MqttConnectionSettings {
        let cs: MqttConnectionSettings;

        try {
            const csElements = connectionString.split(';');
            const csMap = new Map<string, string>();
            for (const element of csElements) {
                const [key, value] = element.split('=');
                csMap.set(key, value);
            }
            const csObj = Object.fromEntries(csMap);

            cs = new MqttConnectionSettings(csObj.HostName);
            cs.clientId = csObj?.ClientId ?? '';
            cs.keyFile = csObj?.KeyFile ?? '';
            cs.certFile = csObj?.CertFile ?? '';
            cs.username = csObj?.Username ?? '';
            cs.password = csObj?.Password ?? '';
            cs.keepAliveInSeconds = Number(csObj?.KeepAliveInSeconds ?? defaultKeepAliveInSeconds);
            cs.cleanSession = Boolean(csObj?.CleanSession === undefined ? defaultCleanSession : csObj?.CleanSession);
            cs.tcpPort = Number(csObj?.TcpPort ?? defaultTcpPort);
            cs.useTls = Boolean(csObj?.UseTls === undefined ? defaultUseTls : csObj?.UseTls);
            cs.caFile = csObj?.CaFile ?? '';
            cs.disableCrl = Boolean(csObj?.DisableCrl === undefined ? defaultDisableCrl : csObj?.DisableCrl);
        }
        catch (ex) {
            throw new Error(`Error while parsing connection string: ${ex.message}`);
        }

        return cs;
    }

    public get hostname(): string {
        return this._hostname;
    }

    public get auth(): AuthType {
        return this.certFile ? AuthType.X509 : AuthType.Basic;
    }
}
