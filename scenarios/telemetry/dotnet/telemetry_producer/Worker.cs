using GeoJSON.Text.Geometry;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using MQTTnet.Extensions.ManagedClient;
using System.Collections.ObjectModel;

namespace telemetry_producer;

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
        var cs = MqttConnectionSettings.CreateFromEnvVars(_configuration.GetValue<string>("envFile"));
        _logger.LogInformation("Connecting to {cs}", cs);

        var mqttClient = new MqttFactory().CreateManagedMqttClient(MqttNetTraceLogger.CreateTraceLogger());

        mqttClient.InternalClient.ConnectedAsync += async cea =>
        {
            _logger.LogInformation("Client {ClientId} connected: {ResultCode}", mqttClient.InternalClient.Options.ClientId, cea.ConnectResult.ResultCode);

            var telemetryPosition = new PositionTelemetryProducer(mqttClient.InternalClient);

            while (!stoppingToken.IsCancellationRequested)
            {
                var pubAck = await telemetryPosition.SendTelemetryAsync(
                    new Point(new Position(51.899523, -2.124156)), stoppingToken);
                _logger.LogInformation("Message published with PUBACK {code}", pubAck.ReasonCode);
                await Task.Delay(5000, stoppingToken);
            }
        };

        await mqttClient!.StartAsync(new ManagedMqttClientOptionsBuilder()
            .WithClientOptions(new MqttClientOptionsBuilder()
                .WithConnectionSettings(cs)
                .WithProtocolVersion(MQTTnet.Formatter.MqttProtocolVersion.V500)
                .Build())
            .WithAutoReconnectDelay(TimeSpan.FromSeconds(5))
            .Build());
    }
}
