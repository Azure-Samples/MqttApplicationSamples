
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Extensions.MultiCloud.Connections;

System.Diagnostics.Trace.Listeners.Add(new System.Diagnostics.ConsoleTraceListener());

var cs = new ConnectionSettings(Environment.GetEnvironmentVariable("Broker")!);
Console.WriteLine($"Connecting to {cs}");

var mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger()) as MqttClient;

var connAck = await mqttClient!.ConnectAsync(new MqttClientOptionsBuilder().WithConnectionSettings(cs).Build());

Console.WriteLine($"Client Connected: {mqttClient.IsConnected} with CONNACK: {connAck.ResultCode}");

mqttClient.ApplicationMessageReceivedAsync += async m => await Console.Out.WriteAsync(
    $"Received message on topic: '{m.ApplicationMessage.Topic}' with content: '{m.ApplicationMessage.ConvertPayloadToString()}'");

await mqttClient.SubscribeAsync("sample/+");
await mqttClient.PublishStringAsync("sample/topic1", "hello world!");

Console.ReadLine();

