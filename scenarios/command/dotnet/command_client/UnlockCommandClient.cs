using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using proto_messages;

namespace command_client;

[RequestTopic("vehicles/{clientId}/command/{commandName}/request")]
[ResponseTopic("vehicles/{clientId}/command/{commandName}/response")]
public class UnlockCommandClient : CommandClient<UnlockRequest, UnlockResponse>
{
    public UnlockCommandClient(IMqttClient mqttClient)
        : base(mqttClient, "unlock", new ProtobufSerializer(UnlockResponse.Parser))
    {
    }
}
