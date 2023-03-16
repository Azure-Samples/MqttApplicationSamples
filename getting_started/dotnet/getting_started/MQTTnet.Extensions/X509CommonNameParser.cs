using System.Diagnostics;
using System.Security.Cryptography.X509Certificates;

namespace MQTTnet.Extensions.MultiCloud.Connections;

public static class X509CommonNameParser
{
    public static string GetCNFromCertSubject(X509Certificate2 cert)
    {
        string result;
        string subject = cert.Subject;
        var dict = subject.ToDictionary(',', '=');
        if (dict.ContainsKey("CN"))
        {
            result = dict["CN"];
        }
        else if (dict.ContainsKey(" CN"))
        {
            result = dict[" CN"];
        }
        else
        {
            Trace.TraceWarning("CN not found in Subject, using thumbprint");
            result = cert.Thumbprint;
        }
        return result;
    }
}
