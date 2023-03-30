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
    public Worker(ILogger<Worker> logger) => _logger = logger;

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var cs = new ConnectionSettings(Environment.GetEnvironmentVariable("Broker")!);
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