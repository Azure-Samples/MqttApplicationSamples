using MQTTnet.Client.Extensions;
using MQTTnet.Client;
using MQTTnet;
using GeoJSON.Text.Geometry;

namespace telemetry_producer;

public class Worker : BackgroundService
{
    private readonly ILogger<Worker> _logger;

    public Worker(ILogger<Worker> logger)
    {
        _logger = logger;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var cs = new ConnectionSettings(Environment.GetEnvironmentVariable("Broker")!);
        Console.WriteLine($"Connecting to {cs}");

        var mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger()) as MqttClient;

        var connAck = await mqttClient!.ConnectAsync(new MqttClientOptionsBuilder().WithConnectionSettings(cs).Build(), stoppingToken);

        Console.WriteLine($"Client Connected: {mqttClient.IsConnected} with CONNACK: {connAck.ResultCode}");

        var telemetryPosition = new Telemetry<Point>(mqttClient);

        while (!stoppingToken.IsCancellationRequested)
        {
            var pubAck = await telemetryPosition.SendMessage(new Point(new Position(51.899523, -2.124156)), stoppingToken);
            _logger.LogInformation("Message published with PUBACK {code}", pubAck.ReasonCode);
            await Task.Delay(5000, stoppingToken);
        }
    }
}
