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
                
                cvArgs.Chain.ChainPolicy.CustomTrustStore.AddRange(caChain);
                Trace.TraceWarning("Validating TLS with chain:\n\t" + string.Join("\n\t",cvArgs.Chain.ChainPolicy.CustomTrustStore.Select(c => c.Subject)));

                cvArgs.Chain.ChainPolicy.VerificationFlags = X509VerificationFlags.IgnoreEndRevocationUnknown | X509VerificationFlags.IgnoreCertificateAuthorityRevocationUnknown;
                Trace.TraceWarning($"Chain validation configured with verification flags:\n\t{cvArgs.Chain.ChainPolicy.VerificationFlags}");

                chainValidated = cvArgs.Chain.Build(new X509Certificate2(cvArgs.Certificate));
                if (chainValidated == false)
                {
                    Trace.TraceError($"Error validating TLS chain for cert: '{cvArgs.Certificate.Subject}' issued by '{cvArgs.Certificate.Issuer}'");
                    cvArgs.Chain.ChainStatus.ToList().ForEach(s => Trace.TraceError("  " + s.StatusInformation));
                }
                return chainValidated;

            }
            Trace.TraceError("RemoteCertificateValidation Errors: " + cvArgs.SslPolicyErrors);
            return false;
        }
    }
}