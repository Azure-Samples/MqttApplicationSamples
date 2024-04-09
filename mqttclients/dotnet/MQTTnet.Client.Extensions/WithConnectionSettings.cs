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
            .WithCleanSession(cs.CleanSession)
            .WithClientId(cs.ClientId)
            .WithProtocolVersion(Formatter.MqttProtocolVersion.V500)
            .WithTlsSettings(cs);

        if (!string.IsNullOrEmpty(cs.Password))
        {
            builder.WithCredentials(cs.Username, cs.Password);
        }

        if (!string.IsNullOrEmpty(cs.PasswordFile))
        {
            builder.WithCredentials(cs.Username, File.ReadAllBytes(cs.PasswordFile));
        }

        return builder;
    }
}

