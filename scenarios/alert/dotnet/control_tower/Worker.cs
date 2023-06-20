using alert_message;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;

namespace control_tower
{
    public class Worker : BackgroundService
    {
        private readonly ILogger<Worker> _logger;
        private readonly IConfiguration _configuration;
        public Worker(ILogger<Worker> logger, IConfiguration configuration)
        {
            _logger = logger;
            _configuration = configuration;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            MqttConnectionSettings cs = MqttConnectionSettings.CreateFromEnvVars(_configuration.GetValue<string>("envFile"));
            _logger.LogInformation("Connecting to {cs}", cs);

            IMqttClient mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger());
            mqttClient.DisconnectedAsync += e =>
            {
                _logger.LogInformation("Mqtt client disconnected with reason: {e}", e.Reason);
                return Task.CompletedTask;
            };

            MqttClientConnectResult connAck = await mqttClient.ConnectAsync(new MqttClientOptionsBuilder().WithConnectionSettings(cs).Build(), stoppingToken);
            _logger.LogInformation("Client {ClientId} connected: {ResultCode}", mqttClient.Options.ClientId, connAck.ResultCode);


            AlertSender alertSender = new(mqttClient);

            bool sendAlert = true;
            while (sendAlert)
            {
                _logger.LogInformation("Sending Weather Alert: {time}", DateTimeOffset.Now);

                await alertSender.SendAlertAsync(new AlertMessage
                {
                    AlertType = AlertType.Weather,
                    AlertText = "Heavy Rain",
                    AlertTime = DateTime.UtcNow
                }, stoppingToken);

                Console.WriteLine("\nSend another alert? (y/n)");
                sendAlert = Console.ReadLine()!.ToLower() == "y";
            }
            _logger.LogInformation("The End");
        }
    }
}