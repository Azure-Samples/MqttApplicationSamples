using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System.Net;
using System.Reflection.PortableExecutable;
using System.Text;
using System.Text.Json;

namespace STSModule;

internal class ModuleBackgroundService : BackgroundService
{
    private ModuleClient? _moduleClient;
    private CancellationToken _cancellationToken;
    private readonly ILogger<ModuleBackgroundService> _logger;
    private readonly IConfiguration _configuration;
    private const string MessageCountConfigKey = "MessageCount";
    private const string SendDataConfigKey = "SendData";
    private const string SendIntervalConfigKey = "SendInterval";
    private static readonly Guid BatchId = Guid.NewGuid();
    private static readonly AtomicBoolean Reset = new(false);
    private static readonly Random Rnd = new();
    private static TimeSpan messageDelay;
    private static bool sendData = true;

    public enum ControlCommandEnum
    {
        Reset = 0,
        NoOperation = 1
    }

    public ModuleBackgroundService(ILogger<ModuleBackgroundService> logger, IConfiguration configuration)
    {
        _logger = logger;
        _configuration = configuration;
    }

    private static bool SendUnlimitedMessages(int maximumNumberOfMessages) => maximumNumberOfMessages < 0;

    protected override async Task ExecuteAsync(CancellationToken cancellationToken)
    {

        messageDelay = _configuration.GetValue("MessageDelay", TimeSpan.FromSeconds(5));
        int messageCount = _configuration.GetValue(MessageCountConfigKey, 500);
        SimulatorParameters simulatorParameters = new()
        {
            MachineTempMin = _configuration.GetValue<double>("machineTempMin", 21),
            MachineTempMax = _configuration.GetValue<double>("machineTempMax", 100),
            MachinePressureMin = _configuration.GetValue<double>("machinePressureMin", 1),
            MachinePressureMax = _configuration.GetValue<double>("machinePressureMax", 10),
            AmbientTemp = _configuration.GetValue<double>("ambientTemp", 21),
            HumidityPercent = _configuration.GetValue("ambientHumidity", 25)
        };

        _logger.LogInformation(
               $"Initializing simulated temperature sensor to send {(SendUnlimitedMessages(messageCount) ? "unlimited" : messageCount.ToString())} "
               + $"messages, at an interval of {messageDelay.TotalSeconds} seconds.\n"
               + $"To change this, set the environment variable {MessageCountConfigKey} to the number of messages that should be sent (set it to -1 to send unlimited messages).");



        _cancellationToken = cancellationToken;

        // Open a connection to the Edge runtime
        _moduleClient = await ModuleClient.CreateFromEnvironmentAsync(); // TODO: settings, new ClientOptions() { ModelId = "dtmi:blah;1", SasTokenTimeToLive = TimeSpan.FromMinutes(60) });

        // Reconnect is not implented because we'll let docker restart the process when the connection is lost
        _moduleClient.MqttClient.ConnectedAsync += (connectedArgs) =>
        {
            _logger.LogInformation($"The client is now connected with authentication method={connectedArgs.ConnectResult.AuthenticationMethod}, an existing session found={connectedArgs.ConnectResult.IsSessionPresent}.");
            return Task.CompletedTask;
        };
        _moduleClient.MqttClient.DisconnectedAsync += (disconnectedArgs) =>
        {
            if (disconnectedArgs.Exception != null)
            {
                _logger.LogInformation("The client has been disconnected. Will exit.");

            }
            else
            {
                _logger.LogWarning($"The client has been ungracefully disconnected due to {disconnectedArgs.Reason}, previously connected={disconnectedArgs.ClientWasConnected}.");
            }
            return Task.CompletedTask;
        };

        try
        {
            await _moduleClient.OpenAsync(cancellationToken);
            await _moduleClient.SetMethodHandlerAsync("reset", ResetMethod, this);

            Twin currentTwinProperties = await _moduleClient.GetTwinAsync();
            if (currentTwinProperties.Properties.Desired.Contains(SendIntervalConfigKey))
            {
                messageDelay = TimeSpan.FromSeconds((int)currentTwinProperties.Properties.Desired[SendIntervalConfigKey]);
            }

            if (currentTwinProperties.Properties.Desired.Contains(SendDataConfigKey))
            {
                sendData = (bool)currentTwinProperties.Properties.Desired[SendDataConfigKey];
                if (!sendData)
                {
                    Console.WriteLine("Sending data disabled. Change twin configuration to start sending again.");
                }
            }

            await _moduleClient.SetDesiredPropertyUpdateCallbackAsync(OnDesiredPropertiesUpdated, _moduleClient);
            await _moduleClient.SetInputMessageHandlerAsync("control", ControlMessageHandle, _moduleClient);
            await SendEvents(_moduleClient, messageCount, simulatorParameters, cancellationToken);
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex.Message);
            while (true)
            {
                Thread.Sleep(1000);
            }
        }
    }

    private static async Task SendEvents(
            ModuleClient moduleClient,
            int messageCount,
            SimulatorParameters sim,
            CancellationToken cts)
    {
        int count = 1;
        double currentTemp = sim.MachineTempMin;
        double normal = (sim.MachinePressureMax - sim.MachinePressureMin) / (sim.MachineTempMax - sim.MachineTempMin);

        while (!cts.IsCancellationRequested && (SendUnlimitedMessages(messageCount) || messageCount >= count))
        {
            if (Reset)
            {
                currentTemp = sim.MachineTempMin;
                Reset.Set(false);
            }

            if (currentTemp > sim.MachineTempMax)
            {
                currentTemp += Rnd.NextDouble() - 0.5; // add value between [-0.5..0.5]
            }
            else
            {
                currentTemp += -0.25 + (Rnd.NextDouble() * 1.5); // add value between [-0.25..1.25] - average +0.5
            }

            if (sendData)
            {
                MessageBody tempData = new()
                {
                    Machine = new Machine
                    {
                        Temperature = currentTemp,
                        Pressure = sim.MachinePressureMin + ((currentTemp - sim.MachineTempMin) * normal),
                    },
                    Ambient = new Ambient
                    {
                        Temperature = sim.AmbientTemp + Rnd.NextDouble() - 0.5,
                        Humidity = Rnd.Next(24, 27)
                    },
                    TimeCreated = DateTime.UtcNow
                };

                string dataBuffer = JsonSerializer.Serialize(tempData);
                Message eventMessage = new(Encoding.UTF8.GetBytes(dataBuffer))
                {
                    ContentEncoding = "utf-8",
                    ContentType = "application/json"
                };
                eventMessage.Properties.Add("sequenceNumber", count.ToString());
                eventMessage.Properties.Add("batchId", BatchId.ToString());
                Console.WriteLine($"\t{DateTime.Now.ToLocalTime()}> Sending message: {count}, Body: [{dataBuffer}]");

                await moduleClient.SendEventAsync("temperatureOutput", eventMessage);
                count++;
            }

            await Task.Delay(messageDelay, cts);
        }

        if (messageCount < count)
        {
            Console.WriteLine($"Done sending {messageCount} messages");
        }
    }

    private static Task<MethodResponse> ResetMethod(MethodRequest methodRequest, object userContext)
    {
        Console.WriteLine("Received direct method call to reset temperature sensor...");
        Reset.Set(true);
        MethodResponse response = new((int)HttpStatusCode.OK);
        return Task.FromResult(response);
    }

    private static async Task OnDesiredPropertiesUpdated(TwinCollection desiredPropertiesPatch, object userContext)
    {
        // At this point just update the configure configuration.
        if (desiredPropertiesPatch.Contains(SendIntervalConfigKey))
        {
            messageDelay = TimeSpan.FromSeconds((int)desiredPropertiesPatch[SendIntervalConfigKey]);
        }

        if (desiredPropertiesPatch.Contains(SendDataConfigKey))
        {
            bool desiredSendDataValue = (bool)desiredPropertiesPatch[SendDataConfigKey];
            if (desiredSendDataValue != sendData && !desiredSendDataValue)
            {
                Console.WriteLine("Sending data disabled. Change twin configuration to start sending again.");
            }

            sendData = desiredSendDataValue;
        }

        ModuleClient moduleClient = (ModuleClient)userContext;
        //var patch = new TwinCollection($"{{ \"SendData\":{sendData.ToString().ToLower()}, \"SendInterval\": {messageDelay.TotalSeconds}}}");
        TwinCollection patch = new()
        {
            ["SendData"] = sendData.ToString().ToLower(),
            ["SendInterval"] = messageDelay.TotalSeconds
        };

        await moduleClient.UpdateReportedPropertiesAsync(patch); // Just report back last desired property.
    }

    private static Task<MessageResponse> ControlMessageHandle(Message message, object userContext)
    {
        byte[] messageBytes = message.GetBytes();
        string messageString = Encoding.UTF8.GetString(messageBytes);

        Console.WriteLine($"Received message Body: [{messageString}]");

        try
        {
            ControlCommand[]? messages = JsonSerializer.Deserialize<ControlCommand[]>(messageString);

            foreach (ControlCommand messageBody in messages!)
            {
                if (messageBody.Command == ControlCommandEnum.Reset)
                {
                    Console.WriteLine("Resetting temperature sensor..");
                    Reset.Set(true);
                }
            }
        }
        catch (JsonException)
        {
            ControlCommand messageBody = JsonSerializer.Deserialize<ControlCommand>(messageString)!;

            if (messageBody.Command == ControlCommandEnum.Reset)
            {
                Console.WriteLine("Resetting temperature sensor..");
                Reset.Set(true);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error: Failed to deserialize control command with exception: [{ex}]");
        }

        return Task.FromResult(MessageResponse.Completed!);
    }

    private class SimulatorParameters
    {
        public double MachineTempMin { get; set; }

        public double MachineTempMax { get; set; }

        public double MachinePressureMin { get; set; }

        public double MachinePressureMax { get; set; }

        public double AmbientTemp { get; set; }

        public int HumidityPercent { get; set; }
    }

    private class ControlCommand
    {
        [System.Text.Json.Serialization.JsonPropertyName("command")]
        public ControlCommandEnum Command { get; set; }
    }
}