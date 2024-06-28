using System.Diagnostics;

namespace MQTTnet.Client.Extensions
{
    public static partial class MqttNetExtensions
    {
        static Func<(byte[], TimeSpan)> _getTokenCallBack = null!;
        static Timer _refreshTimer = null!;


        public static MqttClientOptionsBuilder WithJWT(this MqttClientOptionsBuilder builder, MqttConnectionSettings cs, Func<(byte[], TimeSpan)> getTokenCallBack, IMqttClient mqttClient)
        {
            _getTokenCallBack = getTokenCallBack;

            (byte[] token, TimeSpan ts) = getTokenCallBack();
            Trace.TraceInformation($"Token expires in {ts.TotalSeconds} seconds");
            builder
                .WithConnectionSettings(cs)
                .WithAuthentication("OAUTH2-JWT", token);

            _refreshTimer = new Timer(RefreshToken, mqttClient, 0, (int)ts.TotalSeconds * 1000);
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
                            AuthenticationData = _getTokenCallBack().Item1,
                            ReasonCode = MQTTnet.Protocol.MqttAuthenticateReasonCode.ReAuthenticate
                        });
                });
            }
        }
    }
}
