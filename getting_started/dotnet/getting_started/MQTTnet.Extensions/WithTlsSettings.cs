using MQTTnet.Client;
using System.Security;
using System.Security.Cryptography.X509Certificates;

namespace MQTTnet.Extensions.MultiCloud.Connections;

public static partial class MqttNetExtensions
{
    public static MqttClientOptionsBuilder WithTlsSettings(this MqttClientOptionsBuilder builder, ConnectionSettings cs)
    {
        var tls = new MqttClientOptionsBuilderTlsParameters
        {
            UseTls = cs.UseTls
        };
        if (cs.UseTls)
        {
            var certs = new List<X509Certificate2>();
            if (!string.IsNullOrEmpty(cs.X509Key))
            {
                var cert = X509ClientCertificateLocator.Load(cs.X509Key);
                if (cert.HasPrivateKey == false)
                {
                    throw new SecurityException("Provided Cert Has not Private Key");
                }
                if (string.IsNullOrEmpty(cs.ClientId))
                {
                    cs.ClientId = X509CommonNameParser.GetCNFromCertSubject(cert);
                }
                certs.Add(cert);
            }

            if (!string.IsNullOrEmpty(cs.CaFile))
            {
                X509Certificate2Collection caCerts = new();
                caCerts.ImportFromPemFile(cs.CaFile);
                certs.AddRange(caCerts);
                tls.CertificateValidationHandler = ea => X509ChainValidator.ValidateChain(ea.Certificate, cs.CaFile);
            }
            else
            {
                tls.CertificateValidationHandler += ea => X509ChainValidator.ValidateChain(ea.Certificate);
            }
            tls.Certificates = certs;
            tls.IgnoreCertificateRevocationErrors = cs.DisableCrl;
            builder.WithTls(tls);
        }
        return builder;
    }
}
