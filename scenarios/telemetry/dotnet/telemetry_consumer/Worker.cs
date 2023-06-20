using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;

namespace telemetry_consumer;

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
        MqttConnectionSettings cs = MqttConnectionSettings.CreateFromEnvVars(_configuration.GetValue<string>("envFile")!);
        _logger.LogInformation("Connecting to {cs}", cs);


        IMqttClient mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger());
        mqttClient.DisconnectedAsync += e =>
        {
            _logger.LogInformation("Mqtt client disconnected with reason: {e}", e.Reason);
            return Task.CompletedTask;
        };
        MqttClientConnectResult connAck = await mqttClient.ConnectAsync(new MqttClientOptionsBuilder().WithConnectionSettings(cs).Build(), stoppingToken);
        _logger.LogInformation("Client {ClientId} connected: {ResultCode}", mqttClient.Options.ClientId, connAck.ResultCode);


        PositionTelemetryConsumer positionTelemetry = new(mqttClient)
        {
            OnTelemetryReceived = m =>
            _logger.LogInformation("Received msg from {id}. Coordinates lat: {x}, lon: {y}",
                    m.ClientIdFromTopic,
                    m.Payload!.Coordinates.Latitude,
                    m.Payload.Coordinates.Longitude)
        };
        await positionTelemetry.StartAsync();
    }
}