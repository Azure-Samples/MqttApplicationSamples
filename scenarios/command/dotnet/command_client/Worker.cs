using Google.Protobuf.WellKnownTypes;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;

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

        MqttConnectionSettings cs = MqttConnectionSettings.CreateFromEnvVars(_configuration.GetValue<string>("envFile"));
        _logger.LogInformation("Connecting to {cs}", cs);

        IMqttClient mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger());
        mqttClient.DisconnectedAsync += e =>
        {
            _logger.LogInformation("Mqtt client disconnected with reason: {e}", e.Reason);
            return Task.CompletedTask;
        };
        MqttClientConnectResult connAck = await mqttClient!.ConnectAsync(new MqttClientOptionsBuilder().WithConnectionSettings(cs).Build(), stoppingToken);
        _logger.LogInformation("Client {ClientId} connected: {ResultCode}", mqttClient.Options.ClientId, connAck.ResultCode);

        UnlockCommandClient commandClient = new(mqttClient);

        bool invokeCommand = true;

        while (invokeCommand)
        {
            _logger.LogInformation("Invoking Command: {time}", DateTimeOffset.Now);

            UnlockResponse response = await commandClient.InvokeAsync("vehicle03",
                new UnlockRequest
                {
                    When = DateTime.UtcNow.ToTimestamp(),
                    RequestedFrom = mqttClient.Options.ClientId
                }, 2, stoppingToken);

            _logger.LogInformation("Command response: {res}", response.Succeed);
            Console.WriteLine("\nInvoke command again? (y/n)");
            invokeCommand = Console.ReadLine()!.ToLower() == "y";
        }
        _logger.LogInformation("The End");
    }
}
