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
                .WithConnectionSettings(cs)
                .WithAuthentication("OAUTH2-JWT", getTokenCallBack());

            _refreshTimer = new Timer(RefreshToken, mqttClient, 0, Convert.ToInt32(refreshPeriod.TotalMilliseconds));
            return builder;
        }

        static void RefreshToken(object? state)
        {
            IMqttClient mqttClient = (MqttClient)state!;
            if (mqttClient.IsConnected)
            {
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
