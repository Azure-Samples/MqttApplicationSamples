using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using MQTTnet.Extensions.ManagedClient;

namespace command_server;

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

            UnlockCommandServer commandUnlock = new(mqttClient.InternalClient)
            {
                OnCommandReceived = Unlock
            };
            await commandUnlock.StartAsync();

            await Task.Yield();
        };

        await mqttClient!.StartAsync(new ManagedMqttClientOptionsBuilder()
           .WithClientOptions(new MqttClientOptionsBuilder()
               .WithConnectionSettings(cs)
               .Build())
           .WithAutoReconnectDelay(TimeSpan.FromSeconds(5))
           .Build());
    }

    private async Task<UnlockResponse> Unlock(UnlockRequest unlockRequest)
    {
        _logger.LogInformation("Received Unlock request from {from}", unlockRequest.RequestedFrom);

        return await Task.FromResult(new UnlockResponse
        {
            Succeed = true
        });
    }
}
