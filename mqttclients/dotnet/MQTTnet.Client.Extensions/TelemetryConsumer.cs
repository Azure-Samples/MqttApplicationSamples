namespace MQTTnet.Client.Extensions;

public class TelemetryConsumer<T>
{
    private readonly IMqttClient _mqttClient;
    private readonly IMessageSerializer _serializer;
    private readonly string _topicPattern;
    public Action<TelemetryMessage<T>>? OnTelemetryReceived { get; set; }

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
            await Task.Yield();
            string topic = m.ApplicationMessage.Topic;

            MqttTopicFilterCompareResult res = MqttTopicFilterComparer.Compare(topic, _topicPattern);
            if (res == MqttTopicFilterCompareResult.IsMatch)
            {
                string[] segments = topic.Split('/');
                TelemetryMessage<T> msg = new()
                {
                    ClientIdFromTopic = segments[1],
                    Payload = _serializer.FromBytes<T>(m.ApplicationMessage.PayloadSegment.Array!)
                };
                OnTelemetryReceived?.Invoke(msg);
            }
        };
        await _mqttClient.SubscribeAsync(_topicPattern, Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
    }
}
