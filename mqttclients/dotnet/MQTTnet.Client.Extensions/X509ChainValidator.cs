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

        internal static bool ValidateChain(MqttClientCertificateValidationEventArgs cvArgs, X509Certificate2Collection caChain)
        {
            if (cvArgs.SslPolicyErrors == SslPolicyErrors.None)
            {
                return true;
            }

            if (cvArgs.SslPolicyErrors == SslPolicyErrors.RemoteCertificateChainErrors)
            {
                bool chainValidated = false;

                cvArgs.Chain.ChainPolicy.RevocationFlag = X509RevocationFlag.ExcludeRoot;
                cvArgs.Chain.ChainPolicy.RevocationMode = X509RevocationMode.NoCheck;
                cvArgs.Chain.ChainPolicy.VerificationTime = DateTime.UtcNow;
                cvArgs.Chain.ChainPolicy.TrustMode = X509ChainTrustMode.CustomRootTrust;
                cvArgs.Chain.ChainPolicy.CustomTrustStore.AddRange(caChain);

                X509Certificate cert = cvArgs.Certificate;
                X509Certificate2 x5092 = new(cert);
                chainValidated = cvArgs.Chain.Build(x5092);
                if (chainValidated == false)
                {
                    Trace.TraceError($"Error validating TLS chain for cert: '{cert.Subject}' issued by '{cert.Issuer}'");
                    cvArgs.Chain.ChainStatus.ToList().ForEach(s => Trace.TraceError(s.StatusInformation));
                }
                return chainValidated;
            }
            return false;
        }
    }
}