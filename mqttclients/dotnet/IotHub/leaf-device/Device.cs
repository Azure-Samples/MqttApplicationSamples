using Microsoft.ApplicationInsights;
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System.Text;
using System.Text.Json;

namespace leafdevice
{
    public class Device : BackgroundService
    {
        private readonly ILogger<Device> _logger;
        private readonly IConfiguration _configuration;
        private readonly TelemetryClient _telemetryClient;
        private DeviceClient? _deviceClient;
        private int _interval = 60;
        public Device(ILogger<Device> logger, IConfiguration config, TelemetryClient telClient)
        {
            _logger = logger;
            _configuration = config;
            _telemetryClient = telClient;
        }
        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            string cs = _configuration.GetConnectionString("cs")!;
            _logger.LogInformation("Starting device :{cs}", cs);
            _deviceClient = DeviceClient.CreateFromConnectionString(cs);
            _deviceClient.MqttClient.ConnectedAsync += connectedArgs =>
            {
                _logger.LogInformation("The client is now connected with authentication method={AuthenticationMethod}, an existing session found={IsSessionPresent}.", connectedArgs.ConnectResult.AuthenticationMethod, connectedArgs.ConnectResult.IsSessionPresent);
                return Task.CompletedTask;
            };
            _deviceClient.MqttClient.DisconnectedAsync += disconnectedArgs =>
            {
                if (disconnectedArgs.Exception != null)
                {
                    _logger.LogInformation("The client has been disconnected. Will exit.");

                }
                else
                {
                    _logger.LogWarning("The client has been ungracefully disconnected due to {Reason}, previously connected={ClientWasConnected}.", disconnectedArgs.Reason, disconnectedArgs.ClientWasConnected);
                }
                return Task.CompletedTask;
            };

            await _deviceClient.OpenAsync(stoppingToken);
            await _deviceClient.SetMethodDefaultHandlerAsync(async (req, ctx) =>
            {
                _logger.LogInformation("Method called: {name}", req.Name);
                MethodResponse resp = new(100);
                _telemetryClient.TrackEvent("DM invoked", new Dictionary<string, string> { { "method", req.Name! } });
                switch (req.Name)
                {
                    case "echo":
                        string msg = string.IsNullOrEmpty(req.DataAsJson) ?
                            "" : JsonSerializer.Deserialize<string>(req.DataAsJson)!;
                        resp = new MethodResponse(Encoding.UTF8.GetBytes(JsonSerializer.Serialize(msg + msg)), 200);
                        break;
                    case "setInterval":
                        if (int.TryParse(req.DataAsJson, out int interval))
                        {
                            _interval = interval;
                            resp = new MethodResponse(Encoding.UTF8.GetBytes(JsonSerializer.Serialize(_interval)), 200);
                        }
                        else
                        {
                            resp = new MethodResponse(500);
                        }
                        break;
                    case "getInterval":
                        resp = new MethodResponse(Encoding.UTF8.GetBytes(JsonSerializer.Serialize(_interval)), 200);
                        break;
                    default:
                        resp = new MethodResponse(404);
                        break;
                }
                return await Task.FromResult(resp);
            },
            _deviceClient,
            stoppingToken);

            await _deviceClient.SetMethodHandlerAsync("thrice", (req, ctx) =>
            {
                string msg = string.IsNullOrEmpty(req.DataAsJson) ?
                            "" : JsonSerializer.Deserialize<string>(req.DataAsJson)!;
                return Task.FromResult(new MethodResponse(Encoding.UTF8.GetBytes(JsonSerializer.Serialize(msg + msg + msg)), 200));
            },
            _deviceClient,
            stoppingToken);

            await _deviceClient.SetDesiredPropertyUpdateCallbackAsync(async (twin, ctx) =>
            {
                if (twin.Contains("tick"))
                {
                    object? tick = twin["tick"];
                    if (tick is not null)
                    {
                        TwinCollection reported = new()
                        {
                            ["tick"] = new
                            {
                                av = twin.Version,
                                ad = "tick updated",
                                ac = 200
                            }
                        };

                        await _deviceClient.UpdateReportedPropertiesAsync(reported);
                    }
                }

                if (twin.Contains("reportedPropTest"))
                {
                    string propPrefix = "reportedProp";
                    int count = 5;

                    // Send count reported props for property names reportedProp (0 - count)
                    for (int i = 0; i < count; i++)
                    {
                        TwinCollection props = new()
                        {
                            { $"{propPrefix}-{i}", new Random().Next(100) }
                        };
                        await _deviceClient.UpdateReportedPropertiesAsync(props);
                    }
                    await Task.Delay(500);
                    await _deviceClient.UpdateReportedPropertiesAsync(new TwinCollection() { { $"{propPrefix}-0", null! } });
                }
                _logger.LogInformation("Desired Property received: {t}", twin);
            },
            null!,
            stoppingToken);


            TwinCollection reported = new()
            {
                ["sdkVersion"] = IotHubClient.GetSdkVersion(),
                ["sampleItem"] = new
                {
                    aList = new List<int>() { 1, 2, 3 },
                    ADict = new Dictionary<string, string> { { "myKey", "myValue" } }
                }
            };
            long v = await _deviceClient.UpdateReportedPropertiesAsync(reported, stoppingToken);
            _logger.LogInformation("reported updated to {v}", v);

            Twin twin = await _deviceClient.GetTwinAsync(stoppingToken);
            _logger.LogInformation("twin received: {t}", twin.ToJson());

            while (!stoppingToken.IsCancellationRequested)
            {
                Message msg = new(Encoding.UTF8.GetBytes(JsonSerializer.Serialize(new { Environment.WorkingSet })))
                {
                    ContentEncoding = "utf-8",
                    ContentType = "application/json",
                    ComponentName = "leaf-device",
                    CorrelationId = Guid.NewGuid().ToString(),
                    MessageId = Guid.NewGuid().ToString(),
                    UserId = "rido",
                    CreationTimeUtc = DateTime.UtcNow,
                };
                msg.Properties.Add("myProp", "val4myProp");
                await _deviceClient.SendEventAsync(msg, stoppingToken);
                _logger.LogInformation("telemetry msg sent");
                await Task.Delay(_interval * 1000, stoppingToken);
            }
        }
    }
}