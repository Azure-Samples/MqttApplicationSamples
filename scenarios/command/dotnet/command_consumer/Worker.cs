using Google.Protobuf.WellKnownTypes;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using MQTTnet.Extensions.ManagedClient;

namespace command_consumer;

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
        _logger.LogInformation("Connecting to {cs}", cs);

        var mqttClient = new MqttFactory().CreateManagedMqttClient(MqttNetTraceLogger.CreateTraceLogger());
        CommandClient<unlockRequest, unlockResponse> commandClient = new(mqttClient.InternalClient, "unlock")
        {
            RequestTopicPattern = "vehicles/{clientId}/command/{commandName}/request",
            ResponseTopicPattern = "vehicles/{clientId}/command/{commandName}/response",
        };

        mqttClient.InternalClient.ConnectedAsync += async cea =>
        {
            _logger.LogWarning("Client {ClientId} connected: {ResultCode}", mqttClient.InternalClient.Options.ClientId, cea.ConnectResult.ResultCode);

            while (!stoppingToken.IsCancellationRequested)
            {
                _logger.LogInformation("Invoking Command: {time}", DateTimeOffset.Now);
                unlockResponse response = await commandClient.InvokeAsync("vehicle01",
                    new unlockRequest
                    {
                        When = DateTime.Now.ToUniversalTime().ToTimestamp(),
                        RequestedFrom = mqttClient.InternalClient.Options.ClientId
                    });
                _logger.LogInformation("Command response: {res}", response.Succeed);
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
