"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.SampleMqttClient = void 0;
const logger_1 = require("./logger");
const mqtt_1 = require("mqtt");
const path_1 = require("path");
const fs = require("fs");
const ModuleName = 'SampleMqttClient';
const ConnectTimeoutInSeconds = 10;
class SampleMqttClient {
    get connected() {
        var _a;
        return ((_a = this.mqttClient) === null || _a === void 0 ? void 0 : _a.connected) || false;
    }
    endClientSession() {
        var _a;
        return __awaiter(this, void 0, void 0, function* () {
            if ((_a = this.mqttClient) === null || _a === void 0 ? void 0 : _a.connected) {
                yield this.mqttClient.endAsync(true);
            }
        });
    }
    createMqttClientOptions(connectionSettings) {
        let mqttClientOptions;
        try {
            mqttClientOptions = {
                clientId: connectionSettings.mqttClientId,
                protocol: 'mqtt',
                host: connectionSettings.mqttHostname,
                port: connectionSettings.mqttTcpPort,
                keepalive: connectionSettings.mqttKeepAliveInSeconds,
                connectTimeout: ConnectTimeoutInSeconds * 1000,
                rejectUnauthorized: true,
                manualConnect: true,
                clean: connectionSettings.mqttCleanSession,
                protocolVersion: 5
            };
            if (connectionSettings.mqttUsername) {
                mqttClientOptions.username = connectionSettings.mqttUsername;
                mqttClientOptions.password = connectionSettings.mqttPassword;
            }
            if (connectionSettings.mqttUseTls) {
                mqttClientOptions.protocol = 'mqtts';
            }
            if (connectionSettings.mqttCertFile) {
                mqttClientOptions.cert = fs.readFileSync((0, path_1.resolve)('..', connectionSettings.mqttCertFile));
                mqttClientOptions.key = fs.readFileSync((0, path_1.resolve)('..', connectionSettings.mqttKeyFile));
            }
            if (connectionSettings.mqttCaFile) {
                mqttClientOptions.ca = fs.readFileSync((0, path_1.resolve)('..', connectionSettings.mqttCaFile));
            }
        }
        catch (ex) {
            logger_1.Logger.log([ModuleName, 'error'], `Error while creating client options: ${ex}`);
        }
        return mqttClientOptions;
    }
    connect(connectionSettings) {
        return __awaiter(this, void 0, void 0, function* () {
            try {
                logger_1.Logger.log([ModuleName, 'info'], `Initializing MQTT client`);
                const mqttClientOptions = this.createMqttClientOptions(connectionSettings);
                this.mqttClient = (0, mqtt_1.connect)(mqttClientOptions);
                // Attach MQTT client event handlers
                this.mqttClient.on('connect', this.onConnect.bind(this));
                this.mqttClient.on('disconnect', this.onDisconnect.bind(this));
                this.mqttClient.on('message', this.onMessage.bind(this));
                this.mqttClient.on('error', this.onError.bind(this));
                this.mqttClient.on('close', this.onClose.bind(this));
                this.mqttClient.on('end', this.onEnd.bind(this));
                this.mqttClient.on('reconnect', this.onReconnect.bind(this));
                this.mqttClient.on('offline', this.onOffline.bind(this));
                // Connect to MQTT broker
                logger_1.Logger.log([ModuleName, 'info'], `Starting connection for clientId: ${this.mqttClient.options.clientId}`);
                this.mqttClient.connect();
                yield new Promise((resolve) => {
                    const interval = setInterval(() => {
                        if (this.mqttClient.connected) {
                            clearInterval(interval);
                            return resolve();
                        }
                    }, 1000);
                });
                if (!this.mqttClient.connected) {
                    throw new Error('Unable to connect to MQTT broker');
                }
                logger_1.Logger.log([ModuleName, 'info'], `MQTT client connected - clientId: ${this.mqttClient.options.clientId}`);
            }
            catch (ex) {
                logger_1.Logger.log([ModuleName, 'error'], `MQTT client connect error: ${ex}`);
            }
        });
    }
    subscribe(topic) {
        return __awaiter(this, void 0, void 0, function* () {
            try {
                logger_1.Logger.log([ModuleName, 'info'], `Subscribing to MQTT topics: ${topic}`);
                yield this.mqttClient.subscribeAsync(topic);
            }
            catch (ex) {
                logger_1.Logger.log([ModuleName, 'error'], `MQTT client subscribe error: ${ex}`);
            }
        });
    }
    publish(topic, payload) {
        return __awaiter(this, void 0, void 0, function* () {
            let messageId = -1;
            try {
                logger_1.Logger.log([ModuleName, 'info'], `Publishing to MQTT topic: ${topic}, with payload: ${payload}`);
                // Sending QoS 1 message since we need to return a messageId
                const packet = yield this.mqttClient.publishAsync(topic, payload, { qos: 1 });
                messageId = packet.messageId;
            }
            catch (ex) {
                logger_1.Logger.log([ModuleName, 'error'], `MQTT client publish error: ${ex}`);
            }
            return messageId;
        });
    }
    onConnect(_packet) {
        logger_1.Logger.log([ModuleName, 'info'], `Connected to MQTT broker: ${this.mqttClient.options.host}:${this.mqttClient.options.port}`);
    }
    onDisconnect(_packet) {
        logger_1.Logger.log([ModuleName, 'info'], 'Disconnected from MQTT broker');
    }
    onMessage(topic, payload, _packet) {
        logger_1.Logger.log([ModuleName, 'info'], `Received message on topic: ${topic}, with payload: ${payload.toString('utf8')}`);
    }
    onClose() {
        logger_1.Logger.log([ModuleName, 'info'], 'MQTT broker connection closed');
    }
    onEnd() {
        logger_1.Logger.log([ModuleName, 'info'], 'MQTT broker connection ended');
    }
    onReconnect() {
        logger_1.Logger.log([ModuleName, 'info'], 'MQTT broker session re-connected');
    }
    onOffline() {
        logger_1.Logger.log([ModuleName, 'info'], 'MQTT broker connection is offline');
    }
    onError(error) {
        logger_1.Logger.log([ModuleName, 'error'], `MQTT client error:`);
        if (error === null || error === void 0 ? void 0 : error.code) {
            logger_1.Logger.log([ModuleName, 'error'], `  - reason code: ${error.code}`);
            if (error === null || error === void 0 ? void 0 : error.message) {
                logger_1.Logger.log([ModuleName, 'error'], `  - message: ${error.message}`);
            }
            else {
                const errors = error.errors;
                if (errors && Array.isArray(errors)) {
                    for (const subError of errors) {
                        logger_1.Logger.log([ModuleName, 'error'], `  - message: ${subError.message}`);
                    }
                }
            }
        }
        logger_1.Logger.log([ModuleName, 'error'], `Terminating the sample...`);
        process.exit(1);
    }
}
exports.SampleMqttClient = SampleMqttClient;
//# sourceMappingURL=sampleMqttClient.js.map