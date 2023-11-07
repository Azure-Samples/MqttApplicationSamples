using Azure.Core;
using Azure.Identity;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using System.Text;

MqttConnectionSettings cs = MqttConnectionSettings.CreateFromEnvVars();

var defaultCredential = new DefaultAzureCredential();
AccessToken jwt = defaultCredential.GetToken(new TokenRequestContext(new string[] { "https://eventgrid.azure.net/.default" }));

IMqttClient mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger());
MqttClientConnectResult connAck = await mqttClient!.ConnectAsync(new MqttClientOptionsBuilder()
    .WithJWT(cs, Encoding.UTF8.GetBytes(jwt.Token))
    .Build());

Console.WriteLine($"Client Connected: {mqttClient.IsConnected} with CONNACK: {connAck.ResultCode}");

mqttClient.ApplicationMessageReceivedAsync += async m => await Console.Out.WriteAsync(
    $"Received message on topic: '{m.ApplicationMessage.Topic}' with content: '{m.ApplicationMessage.ConvertPayloadToString()}'\n\n");

MqttClientSubscribeResult suback = await mqttClient.SubscribeAsync("sample/+", MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
suback.Items.ToList().ForEach(s => Console.WriteLine($"subscribed to '{s.TopicFilter.Topic}'  with '{s.ResultCode}'"));

MqttClientPublishResult puback = await mqttClient.PublishStringAsync("sample/topic1", "hello world!", MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
Console.WriteLine(puback.ReasonString);

Console.ReadLine();
