using System.Diagnostics;
using System.Security.Cryptography.X509Certificates;

namespace MQTTnet.Client.Extensions;

public class X509ClientCertificateLocator
{
    public static X509Certificate2 Load(string certFile, string keyFile, string keyFilePassword)
    {
        X509Certificate2? cert = string.IsNullOrEmpty(keyFilePassword) ?
            X509Certificate2.CreateFromPemFile(certFile, keyFile) :
            X509Certificate2.CreateFromEncryptedPemFile(certFile, keyFile, keyFilePassword);

        if (cert.NotAfter.ToUniversalTime() < DateTime.UtcNow)
        {
            throw new ArgumentException($"Cert '{cert.Subject}' expired '{cert.GetExpirationDateString()}'");
        }

        Trace.TraceInformation($"Loaded Cert: {cert.SubjectName.Name} {cert.Thumbprint} issued by {cert.Issuer}, not after: {cert.GetExpirationDateString()}");

        // https://github.com/dotnet/runtime/issues/45680#issuecomment-739912495
        return new X509Certificate2(cert.Export(X509ContentType.Pkcs12)); ;
    }
}
