namespace MQTTnet.Client.Extensions;

public class TelemetryConsumer<T>
{
    readonly IMqttClient _mqttClient;
    readonly IMessageSerializer _serializer;
    readonly string _topicPattern;
    public Func<TelemetryMessage<T>, Task<bool>>? OnTelemetryReceived { get; set; }

    public TelemetryConsumer(IMqttClient mqttClient, IMessageSerializer serializer, string topicPattern)
    {
        _mqttClient = mqttClient;
        _serializer = serializer;
        _topicPattern = topicPattern;
    }

    public async Task StartAsync()
    {
        _mqttClient.ApplicationMessageReceivedAsync += async m =>
        {
            m.AutoAcknowledge = false;
            var topic = m.ApplicationMessage.Topic;

            var res = MqttTopicFilterComparer.Compare(topic, _topicPattern);
            if (res == MqttTopicFilterCompareResult.IsMatch)
            {
                var segments = topic.Split('/');
                var msg = new TelemetryMessage<T>()
                {
                    MessageId = m.PacketIdentifier,
                    ClientIdFromTopic = segments[1],
                    Payload = _serializer.FromBytes<T>(m.ApplicationMessage.PayloadSegment.ToArray())
                };
                bool accept = await OnTelemetryReceived?.Invoke(msg)!;
                System.Diagnostics.Debug.WriteLine($" Msg {m.PacketIdentifier} accepted: {accept}");
                if (accept)
                {
                    await m.AcknowledgeAsync(CancellationToken.None);
                }
            }
        };
        await _mqttClient.SubscribeAsync(_topicPattern, Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
    }
}
