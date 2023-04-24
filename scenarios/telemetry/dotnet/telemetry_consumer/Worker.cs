using GeoJSON.Text.Geometry;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using MQTTnet.Extensions.External.RxMQTT.Client;
using MQTTnet.Extensions.ManagedClient;
using System.Reactive.Linq;

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
        var cs = ConnectionSettings.CreateFromEnvVars(_configuration.GetValue<string>("envFile")!);
        _logger.LogInformation("Connecting to {cs}", cs);

        var mqttClient = new MqttFactory().CreateRxMqttClient(MqttNetTraceLogger.CreateTraceLogger());

        mqttClient.InternalClient.ConnectedAsync += async cea =>
        {
            _logger.LogWarning("Client {ClientId} connected: {ResultCode}", mqttClient.InternalClient.Options.ClientOptions.ClientId, cea.ConnectResult.ResultCode);

            new TelemetryRx<Point>(mqttClient)
                .Start("vehicles/+/position")
                .Subscribe(m => _logger.LogInformation("Received msg from {id}. Coordinates lat: {x}, lon: {y}", 
                    m.ClientIdFromTopic, m.Payload!.Coordinates.Latitude, m.Payload.Coordinates.Longitude));

            await Task.Yield();
        };

        await mqttClient.StartAsync(new ManagedMqttClientOptionsBuilder()
            .WithClientOptions(new MqttClientOptionsBuilder()
                .WithConnectionSettings(cs)
                .Build())
             .WithAutoReconnectDelay(TimeSpan.FromSeconds(5))
            .Build());
    }
}