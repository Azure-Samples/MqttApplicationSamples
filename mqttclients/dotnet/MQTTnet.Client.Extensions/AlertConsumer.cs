namespace MQTTnet.Client.Extensions;

public class AlertConsumer<T>
{
    private readonly IMqttClient _mqttClient;
    private readonly IMessageSerializer _serializer;
    private readonly string _topicPattern;
    public Action<T>? OnAlert { get; set; }

    public AlertConsumer(IMqttClient mqttClient, IMessageSerializer serializer, string topicPattern)
    {
        _mqttClient = mqttClient;
        _serializer = serializer;
        _topicPattern = topicPattern;
    }

    public async Task StartAsync()
    {
        _mqttClient.ApplicationMessageReceivedAsync += m =>
        {
            string topic = m.ApplicationMessage.Topic;

            MqttTopicFilterCompareResult res = MqttTopicFilterComparer.Compare(topic, _topicPattern);
            if (res == MqttTopicFilterCompareResult.IsMatch)
            {
                T alert = _serializer.FromBytes<T>(m.ApplicationMessage.PayloadSegment.Array!);
                OnAlert?.Invoke(alert);
            }
            return Task.CompletedTask;
        };
        await _mqttClient.SubscribeAsync(_topicPattern, Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
    }
}
