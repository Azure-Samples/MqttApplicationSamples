using System.Diagnostics;

namespace MQTTnet.Client.Extensions;

public abstract class CommandClient<T, TResp>
{
    private readonly IMqttClient _mqttClient;
    private readonly string _commandName;
    private TaskCompletionSource<TResp>? _tcs = new();
    private readonly IMessageSerializer _serializer;
    private Guid _correlationId;
    private readonly string _requestTopicPattern;
    private readonly string _responseTopicPattern;
    private string? _responseTopic;

    public CommandClient(IMqttClient mqttClient, string cmdName, IMessageSerializer serializer)
    {
        _mqttClient = mqttClient;
        _commandName = cmdName;
        _serializer = serializer;

        RequestTopicAttribute? requestTopicAttribute = GetType().GetCustomAttributes(true).OfType<RequestTopicAttribute>().FirstOrDefault();
        ArgumentNullException.ThrowIfNull(requestTopicAttribute, nameof(RequestTopicAttribute));

        ResponseTopicAttribute? responseTopicAttribute = GetType().GetCustomAttributes(true).OfType<ResponseTopicAttribute>().FirstOrDefault();
        ArgumentNullException.ThrowIfNull(responseTopicAttribute, nameof(ResponseTopicAttribute));

        _requestTopicPattern = requestTopicAttribute!.Topic;
        _responseTopicPattern = responseTopicAttribute!.Topic;

        _mqttClient.ApplicationMessageReceivedAsync += m =>
        {
            string topic = m.ApplicationMessage.Topic;
            if (topic.Equals(_responseTopic))
            {
                if (m.ApplicationMessage.ContentType != serializer.ContentType)
                {
                    throw new ApplicationException($"Invalid content type. Expected :{_serializer.ContentType} Actual :{m.ApplicationMessage.ContentType}");
                }

                if (_correlationId != new Guid(m.ApplicationMessage.CorrelationData))
                {
                    Trace.TraceWarning($"correlation does not match.Expected {_correlationId} actual {new Guid(m.ApplicationMessage.CorrelationData)}");
                }
                else
                {
                    TResp response = _serializer.FromBytes<TResp>(m.ApplicationMessage.PayloadSegment.Array!);
                    if (!_tcs!.TrySetResult(response))
                    {
                        Trace.TraceError("Cannot set callback");
                    }
                }
            }
            return Task.CompletedTask;
        };
    }
    public async Task<TResp> InvokeAsync(string clientId, T request, int timeoutInMilliSeconds = 5000, CancellationToken ct = default)
    {
        string requestTopic = _requestTopicPattern.Replace("{clientId}", clientId).Replace("{commandName}", _commandName);
        _responseTopic = _responseTopicPattern.Replace("{clientId}", clientId).Replace("{commandName}", _commandName); ;
        _correlationId = Guid.NewGuid();
        await _mqttClient.SubscribeAsync(_responseTopic, Protocol.MqttQualityOfServiceLevel.AtLeastOnce, ct);
        await _mqttClient.PublishAsync(new MqttApplicationMessageBuilder()
            .WithTopic(requestTopic)
            .WithContentType(_serializer.ContentType)
            .WithPayload(_serializer.ToBytes(request!))
            .WithCorrelationData(_correlationId.ToByteArray())
            .WithQualityOfServiceLevel(Protocol.MqttQualityOfServiceLevel.AtLeastOnce)
            .WithResponseTopic(_responseTopic)
            .Build(), ct);

        _tcs = new TaskCompletionSource<TResp>();
        return await _tcs!.Task.TimeoutAfter(TimeSpan.FromMilliseconds(timeoutInMilliSeconds));
    }
}
