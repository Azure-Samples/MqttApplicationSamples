using System.Diagnostics;
using System.Security;
using System.Security.Cryptography.X509Certificates;

namespace MQTTnet.Client.Extensions;

public static partial class MqttNetExtensions
{
    public static MqttClientOptionsBuilder WithTlsSettings(this MqttClientOptionsBuilder builder, MqttConnectionSettings cs)
    {
        if (cs.UseTls)
        {
            var tlsParams = new MqttClientTlsOptionsBuilder();
            tlsParams.WithSslProtocols(System.Security.Authentication.SslProtocols.Tls12 | System.Security.Authentication.SslProtocols.Tls13);

            if (!string.IsNullOrEmpty(cs.CaFile))
            {
                tlsParams.WithCertificateValidationHandler(ea => X509ChainValidator.ValidateChain(ea, cs.CaFile!));
            }

            if (!string.IsNullOrEmpty(cs.CertFile) && !string.IsNullOrEmpty(cs.KeyFile))
            {
                List<X509Certificate2> certs = new();
                X509Certificate2 cert = X509ClientCertificateLocator.Load(cs.CertFile, cs.KeyFile, cs.KeyFilePassword!);
                if (!cert.HasPrivateKey)
                {
                    throw new SecurityException("Provided Cert Has not Private Key");
                }
                certs.Add(cert);
                tlsParams.WithClientCertificates(certs);
            }

            builder.WithTlsOptions(tlsParams.Build());
        }
        return builder;
    }
}

