using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Extensions;
using MQTTnet.Extensions.External.RxMQTT.Client;
using MQTTnet.Extensions.ManagedClient;
using System.Reactive.Linq;

namespace telemetry_consumer
{
    public class Worker : BackgroundService
    {
        private readonly ILogger<Worker> _logger;

        public Worker(ILogger<Worker> logger)
        {
            _logger = logger;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            var cs = new ConnectionSettings(Environment.GetEnvironmentVariable("Broker")!);
            Console.WriteLine($"Connecting to {cs}");

            var mqttClient = new MqttFactory().CreateRxMqttClient();
            await mqttClient.StartAsync(new ManagedMqttClientOptionsBuilder()
                .WithClientOptions(new MqttClientOptionsBuilder()
                    .WithConnectionSettings(cs)
                    .Build())
                .Build());

            Console.WriteLine($"Client Connected: {mqttClient.IsConnected}.");
            
            mqttClient.Connect("vehicles/+/position")
                .Select(m => {
                    string topic = m.ApplicationMessage.Topic;
                    string vid = topic.Split('/')[1];
                    return new { vid, Payload = m.ApplicationMessage.ConvertPayloadToString() };
                })
                .Subscribe(m => _logger.LogInformation($"id: {m.vid} with: {m.Payload}"));

            while (!stoppingToken.IsCancellationRequested) { }
        }

        
    }
}