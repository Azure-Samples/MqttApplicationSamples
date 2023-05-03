using MQTTnet.Client.Extensions;
using MQTTnet;
using MQTTnet.Extensions.ManagedClient;
using MQTTnet.Client;

namespace command_producer;

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
        var cs = ConnectionSettings.CreateFromEnvVars(_configuration.GetValue<string>("envFile"));
        _logger.LogInformation("Connecting to {cs}", cs);

        var mqttClient = new MqttFactory().CreateManagedMqttClient(MqttNetTraceLogger.CreateTraceLogger());

        mqttClient.InternalClient.ConnectedAsync += async cea =>
        {
            _logger.LogWarning("Client {ClientId} connected: {ResultCode}", mqttClient.InternalClient.Options.ClientId, cea.ConnectResult.ResultCode);

            UnlockCommandProducer commandUnlock = new(mqttClient.InternalClient)
            { 
                OnCommandReceived= Unlock
            };
            await Task.Yield();
        };

       await mqttClient!.StartAsync(new ManagedMqttClientOptionsBuilder()
          .WithClientOptions(new MqttClientOptionsBuilder()
              .WithConnectionSettings(cs)
              .Build())
          .WithAutoReconnectDelay(TimeSpan.FromSeconds(5))
          .Build());
    }

    async Task<unlockResponse> Unlock(unlockRequest unlockRequest) 
    {
        _logger.LogWarning("Received Unlock request from {from}", unlockRequest.RequestedFrom);

        return await Task.FromResult(new unlockResponse 
        { 
            Succeed = true
        });
    }
}
