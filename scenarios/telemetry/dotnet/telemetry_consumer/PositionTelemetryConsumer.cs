using GeoJSON.Text.Geometry;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;

namespace telemetry_consumer
{
    internal class PositionTelemetryConsumer : TelemetryConsumer<Point>
    {
        public PositionTelemetryConsumer(IMqttClient mqttClient)
            : base(mqttClient, new Utf8JsonSerializer(), "vehicles/+/position")
        {
        }
    }
}
