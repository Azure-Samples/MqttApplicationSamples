using alert_message;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;

namespace control_tower
{
    internal class AlertSender
    {
        IMqttClient _mqttClient;
        IMessageSerializer _serializer;
        string _alertTopic = "vehicles/weather/alert";

        public AlertSender(IMqttClient mqttClient)
        {
            _mqttClient = mqttClient;
            _serializer = new Utf8JsonSerializer();
        }

        public async Task<MqttClientPublishResult> SendAlertAsync(AlertMessage message, CancellationToken ct = default) =>
            await _mqttClient.PublishAsync(
                new MQTTnet.MqttApplicationMessageBuilder()
                   .WithTopic(_alertTopic)
                   .WithPayload(_serializer.ToBytes(message))
                   .WithQualityOfServiceLevel(MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce)
                   .WithRetainFlag(false)
                   .WithMessageExpiryInterval(300)
                   .Build(),
               ct);
    }
}
