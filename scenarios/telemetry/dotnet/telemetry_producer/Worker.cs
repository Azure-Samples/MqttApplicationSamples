using System.Diagnostics;
using GeoJSON.Text.Geometry;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using MQTTnet.Exceptions;

namespace telemetry_producer;

public class Worker : BackgroundService
{
    private readonly ILogger<Worker> _logger;
    private readonly IConfiguration _configuration;
    private int maxRetryCount = 10;
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
        mqttClient.ConnectedAsync += c => 
        {
            _logger.LogInformation("Client connected. {c}", c.ConnectResult.ResultCode);
            return Task.CompletedTask;
        };
        mqttClient.DisconnectedAsync += async e =>
        {
            _logger.LogWarning("Mqtt client disconnected with reason: {e}, wasConnected {w}", e.Reason, e.ClientWasConnected);

            while (maxRetryCount > 0 && !mqttClient.IsConnected)
            {
                if (maxRetryCount == 1)
                {
                    _logger.LogCritical("Max Retries, exiting");
                    Environment.Exit(-1);
                }

                int delay = (10 - maxRetryCount--) * 1000;
                _logger.LogWarning("Retrying to reconnect, attempt: {c}, delay {d}", 10 - maxRetryCount, delay);
                await Task.Delay(delay);
                await mqttClient.ReconnectAsync(stoppingToken);
                if (mqttClient.IsConnected)
                {
                    maxRetryCount = 10;
                }
                
            }
        };

        MqttClientConnectResult connAck = await mqttClient.ConnectAsync(new MqttClientOptionsBuilder().WithConnectionSettings(cs).Build(), stoppingToken);
        _logger.LogInformation("Client {ClientId} connected: {ResultCode}", mqttClient.Options.ClientId, connAck.ResultCode);


        PositionTelemetryProducer telemetryPosition = new(mqttClient);

        while (!stoppingToken.IsCancellationRequested)
        {
            if (mqttClient.IsConnected)
            {
                MqttClientPublishResult pubAck = await telemetryPosition.SendTelemetryAsync(
                    new Point(new Position(51.899523, -2.124156)), stoppingToken);
                _logger.LogInformation("Message published with PUBACK {code} and mid {mid}", pubAck.ReasonCode, pubAck.PacketIdentifier);
            }
            else
            {
                _logger.LogError("Client is not connected, missing one message");
            }
            await Task.Delay(5000, stoppingToken);

        }
    }
}
