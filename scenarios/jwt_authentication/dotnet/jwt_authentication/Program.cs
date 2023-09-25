using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using System.Text;
using Azure.Identity;
using Azure.Core;

// Create client
IMqttClient mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger());
string hostname = "<Event Grid Mqtt Hostname Here>";

// Create JWT
var defaultCredential = new DefaultAzureCredential();

// Sets the audience field of the JWT to Event Grid
var tokenRequestContext = new TokenRequestContext(new string[] { "https://eventgrid.azure.net/" });
AccessToken jwt = defaultCredential.GetToken(tokenRequestContext);

// Required to use port 8883: https://learn.microsoft.com/azure/event-grid/mqtt-support#code-samples
MqttClientConnectResult connAck = await mqttClient!.ConnectAsync(new MqttClientOptionsBuilder()
    .WithClientId("sample_client")
    .WithTcpServer(hostname, 8883)
    .WithProtocolVersion(MQTTnet.Formatter.MqttProtocolVersion.V500)
    .WithAuthentication("OAUTH2-JWT", Encoding.UTF8.GetBytes(jwt.Token))
    .WithTlsOptions(new MqttClientTlsOptions() { UseTls = true })
    .Build());

Console.WriteLine($"Client Connected: {mqttClient.IsConnected} with CONNACK: {connAck.ResultCode}");

mqttClient.ApplicationMessageReceivedAsync += async m => await Console.Out.WriteAsync(
    $"Received message on topic: '{m.ApplicationMessage.Topic}' with content: '{m.ApplicationMessage.ConvertPayloadToString()}'\n\n");

MqttClientSubscribeResult suback = await mqttClient.SubscribeAsync("sample/+", MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
suback.Items.ToList().ForEach(s => Console.WriteLine($"subscribed to '{s.TopicFilter.Topic}'  with '{s.ResultCode}'"));

MqttClientPublishResult puback = await mqttClient.PublishStringAsync("sample/topic1", "hello world!", MQTTnet.Protocol.MqttQualityOfServiceLevel.AtLeastOnce);
Console.WriteLine(puback.ReasonString);

Console.ReadLine();
