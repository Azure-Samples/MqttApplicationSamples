using Azure.Core;
using Azure.Identity;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using System.Text;

MqttConnectionSettings cs = MqttConnectionSettings.CreateFromEnvVars();

IMqttClient mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger());
MqttClientConnectResult connAck = await mqttClient!.ConnectAsync(new MqttClientOptionsBuilder()
    .WithJWT(cs, GetToken, mqttClient, TimeSpan.FromMinutes(60))
    .Build());

Console.WriteLine($"Client Connected: {mqttClient.IsConnected} with CONNACK: {connAck.ResultCode} with auth method {mqttClient.Options.AuthenticationMethod}");

mqttClient.ApplicationMessageReceivedAsync += async m => await Console.Out.WriteAsync(m.PacketIdentifier.ToString() + ".");
   // $"Received message on topic: '{m.ApplicationMessage.Topic}' with content: {m.ApplicationMessage.ConvertPayloadToString()}");

MqttClientSubscribeResult suback = await mqttClient.SubscribeAsync("sample/+", MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
suback.Items.ToList().ForEach(s => Console.WriteLine($"subscribed to '{s.TopicFilter.Topic}'  with '{s.ResultCode}'"));

int counter = 0;
while (true)
{
    MqttClientPublishResult puback = await mqttClient.PublishStringAsync("sample/topic1", "hello world!" + counter++, MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
    await Task.Delay(100);
}    

static byte[] GetToken()
{
    DefaultAzureCredential defaultCredential = new();
    AccessToken jwt = defaultCredential.GetToken(new TokenRequestContext(new string[] { "https://eventgrid.azure.net/.default" }));
    return Encoding.UTF8.GetBytes(jwt.Token);
}