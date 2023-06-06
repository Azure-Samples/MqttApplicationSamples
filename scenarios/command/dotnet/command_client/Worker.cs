using Google.Protobuf.WellKnownTypes;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using MQTTnet.Extensions.ManagedClient;

namespace command_client;

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
        UnlockCommandClient commandClient = new(mqttClient.InternalClient);

        mqttClient.InternalClient.ConnectedAsync += async cea =>
        {
            _logger.LogInformation("Client {ClientId} connected: {ResultCode}", mqttClient.InternalClient.Options.ClientId, cea.ConnectResult.ResultCode);

            while (!stoppingToken.IsCancellationRequested)
            {
                _logger.LogInformation("Invoking Command: {time}", DateTimeOffset.Now);
                UnlockResponse response = await commandClient.InvokeAsync("vehicle03",
                    new UnlockRequest
                    {
                        When = DateTime.UtcNow.ToTimestamp(),
                        RequestedFrom = mqttClient.InternalClient.Options.ClientId
                    }, 2);
                _logger.LogInformation("Command response: {res}", response.Succeed);
                await Task.Delay(2000, stoppingToken);
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
