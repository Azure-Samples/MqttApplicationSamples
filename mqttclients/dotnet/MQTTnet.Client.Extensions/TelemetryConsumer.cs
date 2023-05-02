using MQTTnet.Client;
using System.Text.RegularExpressions;

namespace MQTTnet.Client.Extensions;

public class TelemetryConsumer<T>
{
    
    public Action<TelemetryMessage<T>>? OnTelemetryReceived { get; set; }

    public TelemetryConsumer(IMqttClient mqttClient, IMessageSerializer serializer, string topicPattern)
    {
        mqttClient.ApplicationMessageReceivedAsync += async m =>
        {
            var topic = m.ApplicationMessage.Topic;

            var res = MqttTopicFilterComparer.Compare(topic, topicPattern);
            if (res == MqttTopicFilterCompareResult.IsMatch)
            {
                var segments = topic.Split('/');
                var msg = new TelemetryMessage<T>()
                {
                    ClientIdFromTopic = segments[1],
                    Payload = serializer.FromBytes<T>(m.ApplicationMessage.Payload)
                };
                OnTelemetryReceived?.Invoke(msg);
            }
            await Task.Yield();
        };
        mqttClient.SubscribeAsync(topicPattern, Protocol.MqttQualityOfServiceLevel.AtLeastOnce).Wait();
    }
}
