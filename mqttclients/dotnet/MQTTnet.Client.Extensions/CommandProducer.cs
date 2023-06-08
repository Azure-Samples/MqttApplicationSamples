using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Server;

namespace MQTTnet.Client.Extensions;

public abstract class CommandProducer<T, TResp>
{
    private readonly IMqttClient _mqttClient;
    private readonly string _commandName;

    public Func<T, Task<TResp>>? OnCommandReceived { get; set; }

    private string _requestTopic = string.Empty;

    private readonly IMessageSerializer _serializer;

    public CommandProducer(IMqttClient mqttClient, string cmdName, IMessageSerializer serializer)
    {
        _mqttClient = mqttClient;
        _commandName = cmdName;
        _serializer = serializer;

        mqttClient.ApplicationMessageReceivedAsync += async m =>
        {
            string topic = m.ApplicationMessage.Topic;
            if (topic.Equals(_requestTopic))
            {
                if (m.ApplicationMessage.ContentType != serializer.ContentType)
                {
                    throw new ApplicationException($"Invalid content type. Expected :{_serializer.ContentType} Actual :{m.ApplicationMessage.ContentType}");
                }

                T request = _serializer.FromBytes<T>(m.ApplicationMessage.Payload);

                TResp response = await OnCommandReceived?.Invoke(request)!;

                byte[] respBytes = _serializer.ToBytes(response);

                MqttClientPublishResult pubAck = await mqttClient.PublishAsync(new MqttApplicationMessageBuilder()
                    .WithTopic(m.ApplicationMessage.ResponseTopic)
                    .WithContentType(_serializer.ContentType)
                    .WithPayload(respBytes)
                    .WithUserProperty("status", 200.ToString())
                    .WithCorrelationData(m.ApplicationMessage.CorrelationData)
                    .Build());
            }
        };
    }
    public string RequestTopicPattern
    {
        set
        {
            _requestTopic = value.Replace("{clientId}", _mqttClient.Options.ClientId).Replace("{commandName}", _commandName);
            _ = _mqttClient.SubscribeAsync(_requestTopic,  MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
        }
    }
}
