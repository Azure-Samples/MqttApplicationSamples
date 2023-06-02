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
    private string _responseTopic = string.Empty;


    public CommandClient(IMqttClient mqttClient, string cmdName, IMessageSerializer serializer)
    {
        _mqttClient = mqttClient;
        _commandName = cmdName;
        _serializer = serializer;

        var rta = GetType().GetCustomAttributes(true).OfType<RequestTopicAttribute>().FirstOrDefault();
        ArgumentNullException.ThrowIfNull(rta, nameof(RequestTopicAttribute));

        _requestTopicPattern = rta!.Topic;

        _mqttClient.ApplicationMessageReceivedAsync += async m =>
        {
            var topic = m.ApplicationMessage.Topic;
            Trace.WriteLine(topic);
            Trace.WriteLine(_tcs!.Task.Status.ToString());
            if (topic.Equals(_responseTopic))
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
                    TResp response = _serializer.FromBytes<TResp>(m.ApplicationMessage.Payload);
                    if (!_tcs!.TrySetResult(response))
                    {
                        Trace.TraceError("Cannot set callback");
                    }
                }
            }
            await Task.Yield();
        };
    }
    public async Task<TResp> InvokeAsync(string clientId, T request, int timeoutInSeconds = 5, CancellationToken ct = default)
    {
        string requestTopic = _requestTopicPattern.Replace("{clientId}", clientId).Replace("{commandName}", _commandName);
        _responseTopic = requestTopic.Replace("request", "response") + "/for/" + _mqttClient.Options.ClientId;
        _correlationId = Guid.NewGuid();
        await _mqttClient.SubscribeAsync(_responseTopic, Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
        await _mqttClient.PublishAsync(new MqttApplicationMessageBuilder()
            .WithTopic(requestTopic)
            .WithContentType(_serializer.ContentType)
            .WithPayload(_serializer.ToBytes(request!))
            .WithCorrelationData(_correlationId.ToByteArray())
            .WithQualityOfServiceLevel(Protocol.MqttQualityOfServiceLevel.AtLeastOnce)
            .WithResponseTopic(_responseTopic)
            .Build());

        _tcs = new TaskCompletionSource<TResp>();
        return await _tcs!.Task.TimeoutAfter(TimeSpan.FromSeconds(timeoutInSeconds));
    }
}
