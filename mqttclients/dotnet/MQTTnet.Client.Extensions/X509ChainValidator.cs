using System.Diagnostics;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;

namespace MQTTnet.Client.Extensions
{
    internal static class X509ChainValidator
    {

        internal static bool ValidateChain(MqttClientCertificateValidationEventArgs certValArgs, string caCertFile = "")
        {
            X509Certificate2Collection caCerts = new();
            if (!string.IsNullOrEmpty(caCertFile))
            {
                caCerts.ImportFromPemFile(caCertFile);
            }
            return ValidateChain(certValArgs, caCerts);
        }

        internal static bool ValidateChain(MqttClientCertificateValidationEventArgs certValArgs, X509Certificate2Collection caChain)
        {
            if (certValArgs.SslPolicyErrors == SslPolicyErrors.None)
            {
                return true;
            }

            Trace.TraceError($"SslPolicyErrors: '{certValArgs.SslPolicyErrors}'");
            Trace.TraceWarning($"Loaded {caChain.Count} certs ");
            caChain.ToList().ForEach(c => Trace.TraceWarning(c.Subject));

            List<SslPolicyErrors> sslErrors = Enum.GetValues<SslPolicyErrors>().Where(e => certValArgs.SslPolicyErrors.HasFlag(e)).ToList();
            bool valid = false;

            foreach (SslPolicyErrors sslError in sslErrors)
            {
                if (sslError == SslPolicyErrors.RemoteCertificateChainErrors)
                {
                    bool chainValidated = false;
                    X509Chain chain = new();
                    chain.ChainPolicy.RevocationFlag = X509RevocationFlag.ExcludeRoot;
                    chain.ChainPolicy.RevocationMode = X509RevocationMode.NoCheck;
                    chain.ChainPolicy.VerificationTime = DateTime.UtcNow;
                    chain.ChainPolicy.UrlRetrievalTimeout = new TimeSpan(0, 0, 10);
                    chain.ChainPolicy.CustomTrustStore.AddRange(caChain);
                    chain.ChainPolicy.TrustMode = X509ChainTrustMode.CustomRootTrust;

                    X509Certificate cert = certValArgs.Certificate;
                    X509Certificate2 x5092 = new(cert);
                    chainValidated = chain.Build(x5092);
                    if (chainValidated == false)
                    {
                        Trace.TraceError($"Error validating TLS chain for cert: '{cert.Subject}' issued by '{cert.Issuer}'");
                        chain.ChainStatus.ToList().ForEach(s => Trace.TraceError(s.StatusInformation));
                    }
                    valid = chainValidated;
                }
                if (sslError == SslPolicyErrors.RemoteCertificateNotAvailable)
                {
                    return false;
                }

                if (sslError == SslPolicyErrors.RemoteCertificateNameMismatch)
                {
                    return false;
                }
            }
            return valid;
        }
    }
}