using MQTTnet.Client;
using System.Text.Json;

namespace telemetry_producer;

internal class Telemetry<T>
{
    private readonly IMqttClient _mqttClient;
    internal string TopicPattern { get; set; } = "vehicles/{clientId}/position";
    public Telemetry(IMqttClient mqttClient)
    {
        _mqttClient = mqttClient;
        TopicPattern = TopicPattern.Replace("{clientId}", mqttClient.Options.ClientId);
    }

    public Task<MqttClientPublishResult> SendMessage(T message, CancellationToken ct = default) => 
        _mqttClient.PublishStringAsync(TopicPattern, JsonSerializer.Serialize(message), MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce, false, ct);
    
}
