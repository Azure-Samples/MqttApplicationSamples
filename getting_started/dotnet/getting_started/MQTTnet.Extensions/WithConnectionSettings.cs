using MQTTnet.Client;

namespace MQTTnet.Extensions.MultiCloud.Connections;

public static partial class MqttNetExtensions
{
    public static MqttClientOptionsBuilder WithConnectionSettings(this MqttClientOptionsBuilder builder, ConnectionSettings cs)
    {
        builder
            .WithTimeout(TimeSpan.FromSeconds(30))
            .WithTcpServer(cs.HostName, cs.TcpPort)
            .WithKeepAlivePeriod(TimeSpan.FromSeconds(cs.KeepAliveInSeconds))
            .WithCleanSession(cs.CleanSession)
            .WithTlsSettings(cs);

        if (!string.IsNullOrEmpty(cs.Password))
        {
            builder.WithCredentials(cs.UserName, cs.Password);
        }

        builder.WithClientId(cs.ClientId);
        return builder;
    }
}

