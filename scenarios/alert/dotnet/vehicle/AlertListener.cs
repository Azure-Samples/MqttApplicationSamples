using alert_message;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;

namespace vehicle
{
    internal class AlertListener : AlertConsumer<AlertMessage>
    {
        public AlertListener(IMqttClient mqttClient)
            : base(mqttClient, new Utf8JsonSerializer(), "vehicles/weather/alert")
        {

        }
    }
}
