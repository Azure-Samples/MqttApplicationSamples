import {
    MqttClient
} from 'mqtt';
import {
    GeoJsonPoint,
    Utf8JsonSerializer,
    TelemetryProducer,
} from '@mqttapplicationsamples/mqttjsclientextensions';

export class PositionTelemetryProducer extends TelemetryProducer<GeoJsonPoint> {
    constructor(mqttClient: MqttClient) {
        super(mqttClient, new Utf8JsonSerializer(), "vehicles/{clientId}/position");
    }
}
