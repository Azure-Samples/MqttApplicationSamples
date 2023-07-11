using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using proto_messages;

namespace command_server;

[RequestTopic("vehicles/{clientId}/command/{commandName}/request")]
public class UnlockCommandServer : CommandServer<UnlockRequest, UnlockResponse>
{
    public UnlockCommandServer(IMqttClient mqttClient)
        : base(mqttClient, "unlock", new ProtobufSerializer(UnlockRequest.Parser))
    {
    }
}
