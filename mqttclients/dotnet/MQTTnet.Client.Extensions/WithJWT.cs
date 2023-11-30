using System.Diagnostics;

namespace MQTTnet.Client.Extensions
{
    public static partial class MqttNetExtensions
    {
        static Func<byte[]> _getTokenCallBack = null!;
        static Timer _refreshTimer = null!;


        public static MqttClientOptionsBuilder WithJWT(this MqttClientOptionsBuilder builder, MqttConnectionSettings cs, Func<byte[]> getTokenCallBack, IMqttClient mqttClient, TimeSpan refreshPeriod)
        {
            _getTokenCallBack = getTokenCallBack;

            builder
                .WithTcpServer(cs.HostName, cs.TcpPort)
                .WithClientId(cs.ClientId)
                .WithProtocolVersion(Formatter.MqttProtocolVersion.V500)
                .WithAuthentication("OAUTH2-JWT", getTokenCallBack());

            if (cs.UseTls)
            {
                builder.WithTlsOptions(new MqttClientTlsOptions() { UseTls = true });
            }
            
            _refreshTimer = new Timer(RefreshToken, mqttClient, Convert.ToInt32(refreshPeriod.TotalMilliseconds), Convert.ToInt32(refreshPeriod.TotalMilliseconds));
            return builder;
        }

        static void RefreshToken(object? state)
        {
            IMqttClient mqttClient = (MqttClient)state!;
            if (mqttClient.IsConnected)
            {
                Console.WriteLine("Refreshing Token " + DateTime.Now.ToString("o"));

                Task.Run(async () =>
                {
                    await mqttClient.SendExtendedAuthenticationExchangeDataAsync(
                        new MqttExtendedAuthenticationExchangeData()
                        {
                            AuthenticationData = _getTokenCallBack(),
                            ReasonCode = MQTTnet.Protocol.MqttAuthenticateReasonCode.ReAuthenticate
                        });
                });
            }
        }
    }
}
