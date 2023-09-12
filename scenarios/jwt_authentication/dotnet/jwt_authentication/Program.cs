using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using System.Text;

//System.Diagnostics.Trace.Listeners.Add(new System.Diagnostics.ConsoleTraceListener());

MqttConnectionSettings cs = MqttConnectionSettings.CreateFromEnvVars();
Console.WriteLine($"Connecting to {cs}");

IMqttClient mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger());
byte[] auth = Encoding.UTF8.GetBytes(File.ReadAllText("auth.json"));

MqttClientConnectResult connAck = await mqttClient!.ConnectAsync(new MqttClientOptionsBuilder()
    .WithConnectionSettings(cs)
    .WithProtocolVersion(MQTTnet.Formatter.MqttProtocolVersion.V500)
    .WithAuthentication("OAUTH2-JWT", auth)
    .Build());

Console.WriteLine($"Client Connected: {mqttClient.IsConnected} with CONNACK: {connAck.ResultCode}");

mqttClient.ApplicationMessageReceivedAsync += async m => await Console.Out.WriteAsync(
    $"Received message on topic: '{m.ApplicationMessage.Topic}' with content: '{m.ApplicationMessage.ConvertPayloadToString()}'\n\n");

MqttClientSubscribeResult suback = await mqttClient.SubscribeAsync("sample/+", MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
suback.Items.ToList().ForEach(s => Console.WriteLine($"subscribed to '{s.TopicFilter.Topic}'  with '{s.ResultCode}'"));

MqttClientPublishResult puback = await mqttClient.PublishStringAsync("sample/topic1", "hello world!", MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
Console.WriteLine(puback.ReasonString);

Console.ReadLine();
