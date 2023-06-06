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
        var cs = MqttConnectionSettings.CreateFromEnvVars(_configuration.GetValue<string>("envFile")!);
        _logger.LogInformation("Connecting to {cs}", cs);

        var mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger());
        var connAck = await mqttClient.ConnectAsync(new MqttClientOptionsBuilder()
            .WithConnectionSettings(cs)
            .Build(), stoppingToken);

        _logger.LogInformation("Client {ClientId} connected: {ResultCode}, session {s}", mqttClient.Options.ClientId, connAck.ResultCode, connAck.IsSessionPresent);

        PositionTelemetryConsumer positionTelemetry = new(mqttClient)
        {
            OnTelemetryReceived = async m =>
            {
                await Task.Yield();

                _logger.LogInformation("Received msg from {id}. Coordinates lat: {x}, lon: {y}",
                    m.ClientIdFromTopic,
                    m.Payload!.Coordinates.Latitude,
                    m.Payload.Coordinates.Longitude);

                return false;
            }
        };
        await positionTelemetry.StartAsync();
    }
}