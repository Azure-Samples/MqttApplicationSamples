using GeoJSON.Text.Geometry;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using MQTTnet.Extensions.ManagedClient;

namespace telemetry_producer;

public class Worker : BackgroundService
{
    private readonly ILogger<Worker> _logger;
    public Worker(ILogger<Worker> logger) => _logger = logger;

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var cs = new ConnectionSettings(Environment.GetEnvironmentVariable("Broker")!);
        _logger.LogInformation("Connecting to {cs}", cs);

        var mqttClient = new MqttFactory().CreateManagedMqttClient(MqttNetTraceLogger.CreateTraceLogger());

        mqttClient.InternalClient.ConnectedAsync += async cea =>
        {
            _logger.LogWarning("Client {ClientId} connected: {ResultCode}", mqttClient.InternalClient.Options.ClientId, cea.ConnectResult.ResultCode);

            var telemetryPosition = new Telemetry<Point>(mqttClient.InternalClient, "vehicles/{clientId}/position");

            while (!stoppingToken.IsCancellationRequested)
            {
                var pubAck = await telemetryPosition.SendMessage(new Point(new Position(51.899523, -2.124156)), stoppingToken);
                _logger.LogInformation("Message published with PUBACK {code}", pubAck.ReasonCode);
                await Task.Delay(5000, stoppingToken);
            }
        };

        await mqttClient!.StartAsync(new ManagedMqttClientOptionsBuilder()
            .WithClientOptions(new MqttClientOptionsBuilder()
                .WithConnectionSettings(cs)
                .Build())
            .WithAutoReconnectDelay(TimeSpan.FromSeconds(5))
            .Build());
    }
}