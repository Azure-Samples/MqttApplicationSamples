using MQTTnet.Client;

namespace MQTTnet.Client.Extensions;

public static partial class MqttNetExtensions
{
    public static MqttClientOptionsBuilder WithConnectionSettings(this MqttClientOptionsBuilder builder, MqttConnectionSettings cs)
    {
        if (string.IsNullOrEmpty(cs.HostName))
        {
            throw new ArgumentNullException(nameof(cs.HostName));
        }
        builder
            .WithTcpServer(cs.HostName, cs.TcpPort)
            .WithKeepAlivePeriod(TimeSpan.FromSeconds(cs.KeepAliveInSeconds))
            .WithCredentials(cs.Username, cs.Password)
            .WithCleanSession(cs.CleanSession)
            .WithTlsSettings(cs);

        builder.WithClientId(cs.ClientId);
        return builder;
    }
}

