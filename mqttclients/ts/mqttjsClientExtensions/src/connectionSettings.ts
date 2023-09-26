import { resolve } from 'path';
import { config } from 'dotenv';

enum AuthType {
    X509 = 'X509',
    Basic = 'Basic'
}

const defaultKeepAliveInSeconds = 30;
const defaultCleanSession = true;
const defaultTcpPort = 8883;
const defaultUseTls = true;
const defaultDisableCrl = false;

export class ConnectionSettings {
    private _hostname: string = '';
    private _clientId: string = '';
    private _certFile: string = '';
    private _keyFile: string = '';
    private _keyFilePassword = '';
    private _username: string = '';
    private _password: string = '';
    private _keepAliveInSeconds: number = defaultKeepAliveInSeconds;
    private _cleanSession: boolean = defaultCleanSession;
    private _tcpPort: number = defaultTcpPort;
    private _useTls: boolean = defaultUseTls;
    private _caFile: string = '';
    private _disableCrl: boolean = defaultDisableCrl;

    private constructor(hostname: string) {
        this._hostname = hostname;
        this._tcpPort = defaultTcpPort;
        this._keepAliveInSeconds = defaultKeepAliveInSeconds;
        this._useTls = defaultUseTls;
        this._disableCrl = defaultDisableCrl;
        this._cleanSession = defaultCleanSession;
    }

    public static fromConnectionString(connectionString: string): ConnectionSettings {
        return ConnectionSettings.parseConnectionString(connectionString);
    }

    public static createFromEnvVars(envFilePath: string = '.env'): ConnectionSettings {
        // Load environment variables from .env file using the dotenv package
        const envConfig = config({ path: envFilePath });

        if (!envConfig.parsed?.MQTT_HOST_NAME) {
            throw new Error('MQTT_HOST_NAME environment variable is not set');
        }

        if (envConfig.parsed?.MQTT_PASSWORD && !envConfig.parsed?.MQTT_USERNAME) {
            throw new Error('MQTT_USERNAME environment variable is required if MQTT_PASSWORD is set');
        }

        const cs = new ConnectionSettings(envConfig.parsed?.MQTT_HOST_NAME);

        cs.tcpPort = Number(envConfig.parsed?.MQTT_TCP_PORT) || 8883;
        cs.useTls = Boolean(envConfig.parsed?.MQTT_USE_TLS === undefined ? true : envConfig.parsed?.MQTT_USE_TLS);
        cs.cleanSession = Boolean(envConfig.parsed?.MQTT_CLEAN_SESSION === undefined ? true : envConfig.parsed?.MQTT_CLEAN_SESSION);
        cs.keepAliveInSeconds = Number(envConfig.parsed?.MQTT_KEEP_ALIVE_IN_SECONDS) || 30;
        cs.clientId = envConfig.parsed?.MQTT_CLIENT_ID || '';
        cs.username = envConfig.parsed?.MQTT_USERNAME || '';
        cs.password = envConfig.parsed?.MQTT_PASSWORD || '';
        cs.certFile = envConfig.parsed?.MQTT_CERT_FILE || '';
        cs.keyFile = envConfig.parsed?.MQTT_KEY_FILE || '';
        cs.caFile = envConfig.parsed?.MQTT_CA_FILE || '';

        return cs;
    }

    public static parseConnectionString(connectionString: string): ConnectionSettings {
        const cs = new ConnectionSettings(connectionString);

        // cs.tcpPort = Number(envConfig.parsed?.MQTT_TCP_PORT) || 8883;
        // cs.useTls = Boolean(envConfig.parsed?.MQTT_USE_TLS === undefined ? true : envConfig.parsed?.MQTT_USE_TLS);
        // cs.cleanSession = Boolean(envConfig.parsed?.MQTT_CLEAN_SESSION === undefined ? true : envConfig.parsed?.MQTT_CLEAN_SESSION);
        // cs.keepAliveInSeconds = Number(envConfig.parsed?.MQTT_KEEP_ALIVE_IN_SECONDS) || 30;
        // cs.clientId = envConfig.parsed?.MQTT_CLIENT_ID || '';
        // cs.username = envConfig.parsed?.MQTT_USERNAME || '';
        // cs.password = envConfig.parsed?.MQTT_PASSWORD || '';
        // cs.certFile = envConfig.parsed?.MQTT_CERT_FILE || '';
        // cs.keyFile = envConfig.parsed?.MQTT_KEY_FILE || '';
        // cs.caFile = envConfig.parsed?.MQTT_CA_FILE || '';

        return cs;
    }

    public get hostname(): string {
        return this._hostname;
    }

    public get clientId(): string {
        return this._clientId;
    }

    public set clientId(value: string) {
        this._clientId = value;
    }

    public get certFile(): string {
        return this._certFile;
    }

    public set certFile(value: string) {
        this._certFile = value;
    }

    public get keyFile(): string {
        return this._keyFile;
    }

    public set keyFile(value: string) {
        this._keyFile = value;
    }

    public get keyFilePassword(): string {
        return this._keyFilePassword;
    }

    public set keyFilePassword(value: string) {
        this._keyFilePassword = value;
    }

    public get auth(): AuthType {
        return this._certFile ? AuthType.X509 : AuthType.Basic;
    }

    public get username(): string {
        return this._username;
    }

    public set username(value: string) {
        this._username = value;
    }

    public get password(): string {
        return this._password;
    }

    public set password(value: string) {
        this._password = value;
    }

    public get keepAliveInSeconds(): number {
        return this._keepAliveInSeconds;
    }

    public set keepAliveInSeconds(value: number) {
        this._keepAliveInSeconds = value;
    }

    public get cleanSession(): boolean {
        return this._cleanSession;
    }

    public set cleanSession(value: boolean) {
        this._cleanSession = value;
    }

    public get tcpPort(): number {
        return this._tcpPort;
    }

    public set tcpPort(value: number) {
        this._tcpPort = value;
    }

    public get useTls(): boolean {
        return this._useTls;
    }

    public set useTls(value: boolean) {
        this._useTls = value;
    }

    public get caFile(): string {
        return this._caFile;
    }

    public set caFile(value: string) {
        this._caFile = value;
    }

    public get disableCrl(): boolean {
        return this._disableCrl;
    }

    public set disableCrl(value: boolean) {
        this._disableCrl = value;
    }
}
