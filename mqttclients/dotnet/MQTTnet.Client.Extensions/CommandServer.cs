namespace MQTTnet.Client.Extensions;

public abstract class CommandServer<T, TResp>
{
    private readonly IMqttClient _mqttClient;
    private readonly string _commandName;

    public Func<T, Task<TResp>>? OnCommandReceived { get; set; }

    private readonly string _requestTopic = string.Empty;

    private readonly IMessageSerializer _serializer;

    public CommandServer(IMqttClient mqttClient, string cmdName, IMessageSerializer serializer)
    {
        _mqttClient = mqttClient;
        _commandName = cmdName;
        _serializer = serializer;

        var rta = GetType().GetCustomAttributes(true).OfType<RequestTopicAttribute>().FirstOrDefault();
        _requestTopic = rta!.Topic.Replace("{clientId}", _mqttClient.Options.ClientId).Replace("{commandName}", _commandName);

        mqttClient.ApplicationMessageReceivedAsync += async m =>
        {
            var topic = m.ApplicationMessage.Topic;
            if (topic.Equals(_requestTopic))
            {
                if (m.ApplicationMessage.ContentType != serializer.ContentType)
                {
                    throw new ApplicationException($"Invalid content type. Expected :{_serializer.ContentType} Actual :{m.ApplicationMessage.ContentType}");
                }

                T request = _serializer.FromBytes<T>(m.ApplicationMessage.Payload);

                TResp response = await OnCommandReceived?.Invoke(request)!;

                byte[] respBytes = _serializer.ToBytes(response);

                var pubAck = await mqttClient.PublishAsync(new MqttApplicationMessageBuilder()
                    .WithTopic(m.ApplicationMessage.ResponseTopic)
                    .WithContentType(_serializer.ContentType)
                    .WithPayload(respBytes)
                    .WithUserProperty("status", 200.ToString())
                    .WithCorrelationData(m.ApplicationMessage.CorrelationData)
                    .Build());
            }
        };
    }

    public async Task StartAsync() => await _mqttClient.SubscribeAsync(_requestTopic, MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);

}
