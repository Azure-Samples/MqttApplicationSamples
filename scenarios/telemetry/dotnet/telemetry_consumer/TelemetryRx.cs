using MQTTnet;
using MQTTnet.Extensions.External.RxMQTT.Client;
using System.Reactive.Linq;
using System.Text.Json;

namespace telemetry_consumer;

internal class TelemetryRx<T>
{
    private readonly IRxMqttClient _mqttClient;

    public TelemetryRx(IRxMqttClient mqttClient)
    {
        _mqttClient = mqttClient;
    }

    public IObservable<TelemetryMessage<T>> Start(string topic)
    {
       return _mqttClient.Connect(topic)
                .Select(m =>
                {
                    string topic = m.ApplicationMessage.Topic;
                    string cid = topic.Split('/')[1];
                    return new TelemetryMessage<T>
                    { 
                        ClientIdFromTopic = cid, 
                        Payload = JsonSerializer.Deserialize<T>(m.ApplicationMessage.ConvertPayloadToString()) 
                    };
                });
    }

}
