using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;

namespace vehicle
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

            MqttClientConnectResult connAck = await mqttClient.ConnectAsync(new MqttClientOptionsBuilder()
                .WithConnectionSettings(cs)
                .WithSessionExpiryInterval(3600)
                .Build(), stoppingToken);
            _logger.LogInformation("Client {ClientId} connected: {ResultCode} with PersistentSession {ps}", mqttClient.Options.ClientId, connAck.ResultCode, connAck.IsSessionPresent);

            AlertListener alertListener = new(mqttClient)
            {
                OnAlert = msg =>
                {
                    _logger.LogInformation("New Alert : Type: {msg}, Text: '{t}' on {d}", msg.AlertType, msg.AlertText, msg.AlertTime);
                }
            };
            await alertListener.StartAsync();
        }
    }
}