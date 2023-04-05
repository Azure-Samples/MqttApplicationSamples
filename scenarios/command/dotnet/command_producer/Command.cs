using Google.Protobuf;
using MQTTnet;
using MQTTnet.Client;
using proto_messages;

namespace command_producer;

internal class Command<T, TResp>
{
    public Func<T, Task<TResp>>? OnMessage { get; set; }

    public string RequestTopicPattern { get; set; } = "vehicles/{clientId}/command/{commandName}/request";
    public string ResponseTopicPattern { get; set; } = "vehicles/{clientId}/command/{commandName}/response";

    private readonly IMessageSerializer _serializer;

    public Command(IMqttClient mqttClient, string cmdName, MessageParser parser)
    {
        _serializer = new ProtobufSerializer(parser);

        string requestTopic = RequestTopicPattern!.Replace("{clientId}", mqttClient.Options.ClientId).Replace("{commandName}", cmdName);
        string responseTopic = ResponseTopicPattern!.Replace("{clientId}", mqttClient.Options.ClientId).Replace("{commandName}", cmdName);

        mqttClient.ApplicationMessageReceivedAsync += async m =>
        {
            var topic = m.ApplicationMessage.Topic;
            if (topic.Equals(requestTopic))
            {
                T request = _serializer.FromBytes<T>(m.ApplicationMessage.Payload);

                TResp response = await OnMessage?.Invoke(request)!;
                
                if (response != null)
                {
                    byte[] respBytes = _serializer.ToBytes(response);
                    var pubAck = await mqttClient.PublishAsync(new MqttApplicationMessageBuilder()
                        .WithTopic(responseTopic)
                        .WithPayload(respBytes)
                        .WithUserProperty("status", 200.ToString())
                        .Build());
                }
            }
        };
        _ = mqttClient.SubscribeAsync(requestTopic);
    }


}
