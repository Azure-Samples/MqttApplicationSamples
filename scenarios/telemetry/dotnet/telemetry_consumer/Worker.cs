using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using MQTTnet.Extensions.ManagedClient;

namespace telemetry_consumer;

public class Worker : BackgroundService
{
    private readonly ILogger<Worker> _logger;
    private readonly IConfiguration _configuration;


    public Worker(ILogger<Worker> logger, IConfiguration configuration)
    {
        _logger = logger;
        _configuration  = configuration;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var cs = MqttConnectionSettings.CreateFromEnvVars(_configuration.GetValue<string>("envFile")!);
        _logger.LogInformation("Connecting to {cs}", cs);

        var mqttClient = new MqttFactory().CreateManagedMqttClient(MqttNetTraceLogger.CreateTraceLogger());

        mqttClient.InternalClient.ConnectedAsync += async cea =>
        {
            _logger.LogInformation("Client {ClientId} connected: {ResultCode}", mqttClient.InternalClient.Options.ClientId, cea.ConnectResult.ResultCode);

            PositionTelemetryConsumer positionTelemetry = new(mqttClient.InternalClient)
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
        };

        await mqttClient.StartAsync(new ManagedMqttClientOptionsBuilder()
            .WithClientOptions(new MqttClientOptionsBuilder()
                .WithConnectionSettings(cs)
                .WithProtocolVersion(MQTTnet.Formatter.MqttProtocolVersion.V500)
                .Build())
             .WithAutoReconnectDelay(TimeSpan.FromSeconds(5))
            .Build());
    }
}