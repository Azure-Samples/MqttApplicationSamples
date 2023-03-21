using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Extensions.External.RxMQTT.Client;
using MQTTnet.Server;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace telemetry_consumer
{
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
}
