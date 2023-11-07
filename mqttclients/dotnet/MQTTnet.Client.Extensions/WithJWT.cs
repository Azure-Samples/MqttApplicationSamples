using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MQTTnet.Client.Extensions
{
    public static partial class MqttNetExtensions
    {
        public static MqttClientOptionsBuilder WithJWT(this MqttClientOptionsBuilder builder, MqttConnectionSettings cs, byte[] token)
        {
            builder
                .WithTcpServer(cs.HostName, cs.TcpPort)
                .WithClientId(cs.ClientId)
                .WithProtocolVersion(Formatter.MqttProtocolVersion.V500)
                .WithAuthentication("OAUTH2-JWT", token);

            if (cs.UseTls)
            {
                builder.WithTlsOptions(new MqttClientTlsOptions() { UseTls = true });
            }

            return builder;
        }
    }
}
