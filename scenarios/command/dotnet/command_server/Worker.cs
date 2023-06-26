using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;

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
        MqttConnectionSettings cs = MqttConnectionSettings.CreateFromEnvVars(_configuration.GetValue<string>("envFile"));
        _logger.LogInformation("Connecting to {cs}", cs);

        IMqttClient mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger());
        mqttClient.DisconnectedAsync += e =>
        {
            _logger.LogInformation("Mqtt client disconnected with reason: {e}", e.Reason);
            return Task.CompletedTask;
        };
        MqttClientConnectResult connAck = await mqttClient.ConnectAsync(new MqttClientOptionsBuilder().WithConnectionSettings(cs).Build(), stoppingToken);
        _logger.LogInformation("Client {ClientId} connected: {ResultCode}", mqttClient.Options.ClientId, connAck.ResultCode);

        UnlockCommandServer commandUnlock = new(mqttClient)
        {
            OnCommandReceived = Unlock
        };
        await commandUnlock.StartAsync(stoppingToken);
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
