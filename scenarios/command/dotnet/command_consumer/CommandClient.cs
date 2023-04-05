using MQTTnet;
using MQTTnet.Client;
using proto_messages;

namespace command_consumer;

internal class CommandClient<T, TResp>
{
    readonly IMqttClient _mqttClient;
    readonly string _commandName;
    string _remoteClientId = string.Empty;
    TaskCompletionSource<TResp>? _tcs;
    IMessageSerializer _serializer;
    Guid _correlationId;

    public string RequestTopicPattern { get; set; } = "device/{clientId}/command/{commandName}/request";
    public string ResponseTopicPattern { get; set; } = "device/{clientId}/command/{commandName}/response";

    public CommandClient(IMqttClient mqttClient, string cmdName)
    {
        _mqttClient = mqttClient;
        _commandName = cmdName;
        _serializer = new ProtobufSerializer(unlockResponse.Parser);
        _mqttClient.ApplicationMessageReceivedAsync += async m =>
        {
            var topic = m.ApplicationMessage.Topic;
            var expectedTopic = ResponseTopicPattern.Replace("{clientId}", _remoteClientId).Replace("{commandName}", cmdName);
            if (topic.Equals(expectedTopic))
            {
                if (m.ApplicationMessage.CorrelationData != null && _correlationId != new Guid(m.ApplicationMessage.CorrelationData))
                {
                    _tcs!.SetException(new ApplicationException("Invalid correlation data"));
                }
                else
                {
                    TResp response = _serializer.FromBytes<TResp>(m.ApplicationMessage.Payload);
                    _tcs!.SetResult(response);
                }
            }
            await Task.Yield();
        };
    }
    public async Task<TResp> InvokeAsync(string clientId, T request, CancellationToken ct = default)
    {
        _remoteClientId = clientId;
        _correlationId = Guid.NewGuid();
        string requestTopic = RequestTopicPattern!.Replace("{clientId}", clientId).Replace("{commandName}", _commandName);
        string responseTopic = ResponseTopicPattern.Replace("{clientId}", clientId).Replace("{commandName}", _commandName);
        _tcs = new TaskCompletionSource<TResp>();
        await _mqttClient.SubscribeAsync(responseTopic);
        await _mqttClient.PublishAsync(new MqttApplicationMessageBuilder()
            .WithTopic(requestTopic)
            .WithPayload(_serializer.ToBytes(request!))
            .WithCorrelationData(_correlationId.ToByteArray())
            .Build());
        return await _tcs.Task;
    }
}
