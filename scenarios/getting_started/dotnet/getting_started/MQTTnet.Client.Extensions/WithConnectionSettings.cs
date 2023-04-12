using MQTTnet.Client;

namespace MQTTnet.Client.Extensions;

public static partial class MqttNetExtensions
{
    public static MqttClientOptionsBuilder WithConnectionSettings(this MqttClientOptionsBuilder builder, ConnectionSettings cs)
    {
        builder
            .WithTcpServer(cs.HostName, cs.TcpPort)
            .WithKeepAlivePeriod(TimeSpan.FromSeconds(cs.KeepAliveInSeconds))
            .WithCredentials(cs.UserName, cs.Password)
            .WithCleanSession(cs.CleanSession)
            .WithProtocolVersion(Formatter.MqttProtocolVersion.V500)
            .WithTlsSettings(cs);

        builder.WithClientId(cs.ClientId);
        return builder;
    }
}

