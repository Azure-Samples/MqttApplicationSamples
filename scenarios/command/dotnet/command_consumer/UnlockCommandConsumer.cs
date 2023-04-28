using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
namespace command_consumer;

public class UnlockCommandConsumer : CommandConsumer<unlockRequest, unlockResponse>
{
    public UnlockCommandConsumer(IMqttClient mqttClient)
        : base(mqttClient, "unlock", new ProtobufSerializer(unlockResponse.Parser))
    {
        RequestTopicPattern = "vehicles/{clientId}/command/{commandName}/request";
        ResponseTopicPattern = "vehicles/{clientId}/command/{commandName}/response";
    }
}
