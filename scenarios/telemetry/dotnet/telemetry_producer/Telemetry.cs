using MQTTnet.Client;
using System.Text.Json;

namespace telemetry_producer;

internal class Telemetry<T>
{
    private readonly IMqttClient _mqttClient;
    private readonly string _topic;
    public Telemetry(IMqttClient mqttClient, string topic)
    {
        _mqttClient = mqttClient;
        _topic = topic.Replace("{clientId}", mqttClient.Options.ClientId);
    }

    public Task<MqttClientPublishResult> SendMessage(T message, CancellationToken ct = default) => 
        _mqttClient.PublishStringAsync(_topic, JsonSerializer.Serialize(message), MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce, false, ct);
    
}
