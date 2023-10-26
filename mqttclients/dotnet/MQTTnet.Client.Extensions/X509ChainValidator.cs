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
                cvArgs.Chain.Reset();
                cvArgs.Chain.ChainPolicy.TrustMode = X509ChainTrustMode.CustomRootTrust;
                cvArgs.Chain.ChainPolicy.VerificationFlags = X509VerificationFlags.IgnoreEndRevocationUnknown;
                cvArgs.Chain.ChainPolicy.CustomTrustStore.AddRange(caChain);
                chainValidated = cvArgs.Chain.Build(new X509Certificate2(cvArgs.Certificate));
                if (chainValidated == false)
                {
                    Trace.TraceError($"Error validating TLS chain for cert: '{cvArgs.Certificate.Subject}' issued by '{cvArgs.Certificate.Issuer}'");
                    cvArgs.Chain.ChainStatus.ToList().ForEach(s => Trace.TraceError(s.StatusInformation));
                }
                return chainValidated;
            }
            Trace.TraceError("RemoteCertificateValidation error:" + cvArgs.SslPolicyErrors);
            return false;
        }
    }
}