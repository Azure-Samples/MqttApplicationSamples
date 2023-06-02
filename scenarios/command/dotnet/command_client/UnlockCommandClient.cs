using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using proto_messages;

namespace command_client;

[RequestTopic("vehicles/{clientId}/command/{commandName}/request")]
public class UnlockCommandClient : CommandClient<UnlockRequest, UnlockResponse>
{
    public UnlockCommandClient(IMqttClient mqttClient)
        : base(mqttClient, "unlock", new ProtobufSerializer(UnlockResponse.Parser))
    {
    }
}
