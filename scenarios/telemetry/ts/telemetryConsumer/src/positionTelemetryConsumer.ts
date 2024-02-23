import {
    MqttClient
} from 'mqtt';
import {
    GeoJsonPoint,
    Utf8JsonSerializer,
    TelemetryConsumer
} from '@mqttapplicationsamples/mqttjsclientextensions';

export class PositionTelemetryConsumer extends TelemetryConsumer<GeoJsonPoint> {
    constructor(mqttClient: MqttClient) {
        super(mqttClient, new Utf8JsonSerializer(), "vehicles/+/position");
    }
}
