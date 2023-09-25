"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.ConnectionSettings = void 0;
const path_1 = require("path");
const dotenv_1 = require("dotenv");
(0, dotenv_1.config)({ path: (0, path_1.resolve)(__dirname, '../../.env') });
class ConnectionSettings {
    constructor() {
        // ...
    }
    static get instance() {
        /* eslint-disable no-underscore-dangle */
        if (!ConnectionSettings._instance) {
            ConnectionSettings._instance = new ConnectionSettings();
        }
        return ConnectionSettings._instance;
        /* eslint-disable no-underscore-dangle */
    }
    get MQTT_HOST_NAME() {
        return process.env.MQTT_HOST_NAME || '';
    }
    get MQTT_TCP_PORT() {
        return Number(process.env.MQTT_TCP_PORT) || 8883;
    }
    get MQTT_USE_TLS() {
        return Boolean(process.env.MQTT_USE_TLS === undefined ? true : process.env.MQTT_USE_TLS);
    }
    get MQTT_CLEAN_SESSION() {
        return Boolean(process.env.MQTT_CLEAN_SESSION === undefined ? true : process.env.MQTT_CLEAN_SESSION);
    }
    get MQTT_KEEP_ALIVE_IN_SECONDS() {
        return Number(process.env.MQTT_KEEP_ALIVE_IN_SECONDS) || 30;
    }
    get MQTT_CLIENT_ID() {
        return process.env.MQTT_CLIENT_ID || '';
    }
    get MQTT_USERNAME() {
        return process.env.MQTT_USERNAME || '';
    }
    get MQTT_PASSWORD() {
        return process.env.MQTT_PASSWORD || '';
    }
    get MQTT_CA_FILE() {
        return process.env.MQTT_CA_FILE || '';
    }
    get MQTT_CERT_FILE() {
        return process.env.MQTT_CERT_FILE || '';
    }
    get MQTT_KEY_FILE() {
        return process.env.MQTT_KEY_FILE || '';
    }
    get MQTT_KEY_FILE_PASSWORD() {
        return process.env.MQTT_KEY_FILE_PASSWORD || '';
    }
}
exports.ConnectionSettings = ConnectionSettings;
//# sourceMappingURL=connectionSettings.js.map