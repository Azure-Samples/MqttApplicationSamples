using MQTTnet;
using MQTTnet.Extensions.External.RxMQTT.Client;
using System.Reactive.Linq;

namespace telemetry_consumer;

internal class TelemetryMessage
{
    public string? ClientIdFromTopic { get; set; }
    public string? PayloadString { get; set; }
}


internal class TelemetryRx
{
    private readonly IRxMqttClient _mqttClient;

    public TelemetryRx(IRxMqttClient mqttClient)
    {
        _mqttClient = mqttClient;
    }

    public IObservable<TelemetryMessage> Start(string topic)
    {
       return _mqttClient.Connect(topic)
                .Select(m =>
                {
                    string topic = m.ApplicationMessage.Topic;
                    string cid = topic.Split('/')[1];
                    return new TelemetryMessage
                    { 
                        ClientIdFromTopic = cid, 
                        PayloadString = m.ApplicationMessage.ConvertPayloadToString() 
                    };
                });
    }

}
