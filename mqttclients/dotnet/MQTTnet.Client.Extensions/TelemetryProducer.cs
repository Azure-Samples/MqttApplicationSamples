
using MQTTnet.Protocol;

namespace MQTTnet.Client.Extensions;

public class TelemetryProducer<T>
{
    private readonly IMqttClient _mqttClient;
    private readonly string _telemetryTopic;
    private readonly IMessageSerializer _serializer;

    public TelemetryProducer(IMqttClient mqttClient, IMessageSerializer serializer, string topicPattern)
    {
        _mqttClient = mqttClient;
        _serializer = serializer;
        _telemetryTopic = topicPattern!.Replace("{clientId}", _mqttClient.Options.ClientId);
    }

    public Task<MqttClientPublishResult> SendTelemetryAsync(
        T message,
        CancellationToken ct = default) =>
            SendTelemetryAsync(message, MqttQualityOfServiceLevel.AtLeastOnce, false, ct);

    public Task<MqttClientPublishResult> SendTelemetryAsync(
        T message,
        MqttQualityOfServiceLevel qos = MqttQualityOfServiceLevel.AtLeastOnce,
        bool retain = false,
        CancellationToken ct = default) =>
            _mqttClient.PublishBinaryAsync(
                _telemetryTopic,
                _serializer.ToBytes(message),
                qos,
                retain,
                ct);
}
