using MQTTnet;
using MQTTnet.Client;
using System.Diagnostics;

namespace MQTTnet.Client.Extensions;

public abstract class CommandConsumer<T, TResp>
{
    private readonly IMqttClient _mqttClient;
    private readonly string _commandName;
    private string _remoteClientId = string.Empty;
    private TaskCompletionSource<TResp>? _tcs = new();
    private readonly IMessageSerializer _serializer;
    private Guid _correlationId;

    public string? RequestTopicPattern { get; set; }
    public string? ResponseTopicPattern { get; set; }

    public CommandConsumer(IMqttClient mqttClient, string cmdName, IMessageSerializer serializer)
    {
        _mqttClient = mqttClient;
        _commandName = cmdName;
        _serializer = serializer;
        _mqttClient.ApplicationMessageReceivedAsync += async m =>
        {
            var topic = m.ApplicationMessage.Topic;
            Trace.WriteLine(topic);
            Trace.WriteLine(_tcs!.Task.Status.ToString());
            var expectedTopic = ResponseTopicPattern!.Replace("{clientId}", _remoteClientId).Replace("{commandName}", cmdName);
            if (topic.Equals(expectedTopic))
            {
                if (m.ApplicationMessage.ContentType != serializer.ContentType)
                {
                    throw new ApplicationException($"Invalid content type. Expected :{_serializer.ContentType} Actual :{m.ApplicationMessage.ContentType}");
                }

                if (_correlationId != new Guid(m.ApplicationMessage.CorrelationData))
                {
                    Trace.TraceWarning($"correlation does not match.Expected {_correlationId} actual s{new Guid(m.ApplicationMessage.CorrelationData)}");
                }
                else
                {
                    TResp response = _serializer.FromBytes<TResp>(m.ApplicationMessage.PayloadSegment.ToArray());
                    if (!_tcs!.TrySetResult(response))
                    {
                        Trace.TraceError("Cannot set callback");
                    }
                }
            }
            await Task.Yield();
        };
    }
    public Task<TResp> InvokeAsync(string clientId, T request, int timeoutInSeconds = 5, CancellationToken ct = default)
    {
        _remoteClientId = clientId;
        _correlationId = Guid.NewGuid();
        string requestTopic = RequestTopicPattern!.Replace("{clientId}", clientId).Replace("{commandName}", _commandName);
        string responseTopic = ResponseTopicPattern!.Replace("{clientId}", clientId).Replace("{commandName}", _commandName);
        _ = _mqttClient.SubscribeAsync(responseTopic, MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
        _ = _mqttClient.PublishAsync(new MqttApplicationMessageBuilder()
            .WithTopic(requestTopic)
            .WithContentType(_serializer.ContentType)
            .WithPayload(_serializer.ToBytes(request!))
            .WithCorrelationData(_correlationId.ToByteArray())
            .WithQualityOfServiceLevel( MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce)
            .WithResponseTopic(responseTopic)
            .Build());

        _tcs = new TaskCompletionSource<TResp>();
        return _tcs!.Task.TimeoutAfter(TimeSpan.FromSeconds(timeoutInSeconds));
    }
}
