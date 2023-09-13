using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System.Text;
using System.Text.Json;

namespace module_sample
{
    public class Module : BackgroundService
    {
        private readonly ILogger<Module> _logger;
        private int interval = 60;
        public Module(ILogger<Module> logger) => _logger = logger;

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            ModuleClient moduleClient = await ModuleClient.CreateFromEnvironmentAsync();
            await moduleClient.OpenAsync(stoppingToken);
            _logger.LogInformation("Module connected with SDK: {av}", ModuleClient.GetSdkVersion());

            await moduleClient.SetInputMessageHandlerAsync("input1", async (msg, ctx) =>
            {
                _logger.LogInformation("Message received on input1: {msg}", Encoding.UTF8.GetString(msg.GetBytes()));
                return await Task.FromResult(MessageResponse.Completed!);
            }, this, stoppingToken);

            await moduleClient.SetMethodDefaultHandlerAsync(async (req, ctx) =>
            {
                _logger.LogInformation("Method {n} invoked", req.Name);
                MethodResponse response = new(200);
                switch (req.Name)
                {
                    case "echo":
                        string? reqString = JsonSerializer.Deserialize<string>(req.DataAsJson);
                        byte[] respBytes = Encoding.UTF8.GetBytes(JsonSerializer.Serialize(reqString + reqString));
                        response = new MethodResponse(respBytes, 200);
                        break;
                    case "getInterval":
                        byte[] respBytes2 = Encoding.UTF8.GetBytes(JsonSerializer.Serialize(interval));
                        response = new MethodResponse(respBytes2, 200);
                        break;
                    case "setInterval":
                        byte[] respBytes3 = Encoding.UTF8.GetBytes(JsonSerializer.Serialize(interval));
                        int newInterval = JsonSerializer.Deserialize<int>(req.DataAsJson);
                        interval = newInterval;
                        response = new MethodResponse(respBytes3, 200);
                        break;
                }
                return await Task.FromResult(response);

            }, this, stoppingToken);

            await moduleClient.SetMethodHandlerAsync("thrice", (req, ctx) =>
            {
                string msg = string.IsNullOrEmpty(req.DataAsJson) ?
                            "" : JsonSerializer.Deserialize<string>(req.DataAsJson)!;
                return Task.FromResult(new MethodResponse(Encoding.UTF8.GetBytes(JsonSerializer.Serialize(msg + msg + msg)), 200));
            },
            moduleClient,
            stoppingToken);

            await moduleClient.SetDesiredPropertyUpdateCallbackAsync(async (twin, ctx) =>
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

                        await moduleClient.UpdateReportedPropertiesAsync(reported);
                    }
                }
                if (twin.Contains("reportedPropTest"))
                {
                    string propPrefix = "reportedProp";
                    int count = 5;

                    // Send count reported props for property names reportedProp (0 - count)
                    for (int i = 0; i < count; i++)
                    {
                        TwinCollection props = new();
                        props.Add($"{propPrefix}-{i}", new Random().Next(100));
                        await moduleClient.UpdateReportedPropertiesAsync(props);
                    }
                    await Task.Delay(500);
                    await moduleClient.UpdateReportedPropertiesAsync(new TwinCollection() { { $"{propPrefix}-0", null! } });
                }
                _logger.LogInformation("Desired Property received: {t}", twin);
            }, null!, cancellationToken: stoppingToken);

            Twin twin = await moduleClient.GetTwinAsync(stoppingToken);
            _logger.LogInformation("twin received: {t}", twin.ToJson());

            while (!stoppingToken.IsCancellationRequested)
            {
                Message msg = new(Encoding.UTF8.GetBytes(JsonSerializer.Serialize(new { Environment.WorkingSet })))
                {
                    ContentEncoding = "utf-8",
                    ContentType = "application/json"
                };
                await moduleClient.SendEventAsync(msg, stoppingToken);
                _logger.LogInformation("Telemetry Sent: {time}", DateTimeOffset.Now);
                await Task.Delay(interval * 1000, stoppingToken);
            }
        }
    }
}