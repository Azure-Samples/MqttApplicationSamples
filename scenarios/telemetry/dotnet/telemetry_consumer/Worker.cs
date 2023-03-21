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

    public Worker(ILogger<Worker> logger)
    {
        _logger = logger;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var cs = new ConnectionSettings(Environment.GetEnvironmentVariable("Broker")!);
        Console.WriteLine($"Connecting to {cs}");

        var mqttClient = new MqttFactory().CreateRxMqttClient();

        mqttClient.InternalClient.ConnectedAsync += async cea =>
        {
            Console.WriteLine($"Client {mqttClient.InternalClient.Options.ClientOptions.ClientId} Connected: {mqttClient.IsConnected}.");

            new TelemetryRx(mqttClient)
                .Start("vehicles/+/position")
                .Subscribe(m => _logger.LogInformation("{id} sent {msg}", m.ClientIdFromTopic, m.PayloadString));

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