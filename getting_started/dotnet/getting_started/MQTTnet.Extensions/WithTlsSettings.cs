﻿using MQTTnet.Client;
using System.Security;
using System.Diagnostics;
using System.Security.Cryptography.X509Certificates;

namespace MQTTnet.Client.Extensions;

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
            X509Certificate2 cert;
            if (!string.IsNullOrEmpty(cs.X509Key))
            {
                cert = X509ClientCertificateLocator.Load(cs.X509Key);
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
                foreach (var c in caCerts) Trace.WriteLine($"cert trust chain: {c.Subject}");
                tls.CertificateValidationHandler = ea => X509ChainValidator.ValidateChain(ea.Certificate, caCerts);
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