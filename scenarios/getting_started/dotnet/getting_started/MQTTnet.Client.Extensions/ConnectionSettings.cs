using System.Text;

namespace MQTTnet.Client.Extensions;

public enum AuthType
{
    X509,
    Basic
}

public class ConnectionSettings
{
    private const int Default_KeepAliveInSeconds = 60;
    private const string Default_CleanSession = "true";
    private const int Default_TcpPort = 8883;
    private const string Default_UseTls = "true";
    private const string Default_DisableCrl = "false";

    public string? HostName { get; set; }
    public string? ClientId { get; set; }
    public string? X509KeyPath { get; set; } //paht-to.pfx|pfxpwd, or thumbprint
    public string? X509PemPath {get ;set; }

    public AuthType Auth
    {
        get => !string.IsNullOrEmpty(X509PemPath) ? AuthType.X509 : AuthType.Basic;
    }
    public string? UserName { get; set; }
    public string? Password { get; set; }
    public int KeepAliveInSeconds { get; set; }
    public bool CleanSession { get; set; }
    public int TcpPort { get; set; }
    public bool UseTls { get; set; }
    public string? CaFile { get; set; }
    public bool DisableCrl { get; set; }


    public ConnectionSettings()
    {
        TcpPort = Default_TcpPort;
        KeepAliveInSeconds = Default_KeepAliveInSeconds;
        UseTls = Default_UseTls == "true";
        DisableCrl = Default_DisableCrl == "true";
        CleanSession = Default_CleanSession == "true";
    }

    public static ConnectionSettings FromConnectionString(string cs) => new(cs);
    public ConnectionSettings(string cs) => ParseConnectionString(cs);

    public static ConnectionSettings CreateFromEnvVars()
    {
        if (File.Exists(".env"))
        {
            foreach (var line in File.ReadAllLines(".env"))
            {
                var parts = line.Split(
                    '=',
                    StringSplitOptions.RemoveEmptyEntries);

                if (parts.Length != 2)
                    continue;

                Environment.SetEnvironmentVariable(parts[0], parts[1]);
            }
        }
        
        string Env(string name) => System.Environment.GetEnvironmentVariable(name) ?? string.Empty;
        return new ConnectionSettings
        {
            HostName = Env(nameof(HostName)),
            ClientId = Env(nameof(ClientId)),
            X509PemPath = Env(nameof(X509PemPath)),
            X509KeyPath = Env(nameof(X509KeyPath)),
            UserName = Env(nameof(UserName)),
            Password = Env(nameof(Password)),
            KeepAliveInSeconds = int.TryParse(Env(nameof(KeepAliveInSeconds)), out int keepAliveInSeconds) ? keepAliveInSeconds : Default_KeepAliveInSeconds,
            CleanSession = Env(nameof(CleanSession)) == "true",
            TcpPort = int.TryParse(Env(nameof(TcpPort)), out int tcpPort) ? tcpPort : Default_TcpPort,
            UseTls = string.IsNullOrEmpty(Env(nameof(UseTls))) ? Default_UseTls == "true" : Env(nameof(UseTls)) == "true",
            CaFile = Env(nameof(CaFile)),
            DisableCrl = Env(nameof(DisableCrl)) == "true"
        };
       
    }

    private static string GetStringValue(IDictionary<string, string> dict, string propertyName, string defaultValue = "")
    {
        string result = defaultValue;
        if (dict.TryGetValue(propertyName, out string? value))
        {
            result = value;
        }
        return result;
    }

    private static int GetPositiveIntValueOrDefault(IDictionary<string, string> dict, string propertyName, int defaultValue)
    {
        int result = defaultValue;
        if (dict.TryGetValue(propertyName, out string? stringValue))
        {
            if (int.TryParse(stringValue, out int intValue))
            {
                result = intValue;
            }
        }
        return result;
    }

    private void ParseConnectionString(string cs)
    {
        IDictionary<string, string> map = cs.ToDictionary(';', '=');
        HostName = GetStringValue(map, nameof(HostName));
        ClientId = GetStringValue(map, nameof(ClientId));
        X509KeyPath = GetStringValue(map, nameof(X509KeyPath));
        X509PemPath = GetStringValue(map, nameof(X509PemPath));
        UserName = GetStringValue(map, nameof(UserName));
        Password = GetStringValue(map, nameof(Password));
        KeepAliveInSeconds = GetPositiveIntValueOrDefault(map, nameof(KeepAliveInSeconds), Default_KeepAliveInSeconds);
        CleanSession = GetStringValue(map, nameof(CleanSession), Default_CleanSession) == "true";
        TcpPort = GetPositiveIntValueOrDefault(map, nameof(TcpPort), Default_TcpPort);
        UseTls = GetStringValue(map, nameof(UseTls), Default_UseTls) == "true";
        CaFile = GetStringValue(map, nameof(CaFile));
        DisableCrl = GetStringValue(map, nameof(DisableCrl), Default_DisableCrl) == "true";
    }

    private static void AppendIfNotEmpty(StringBuilder sb, string name, string val)
    {
        if (!string.IsNullOrEmpty(val))
        {
            if (name.Contains("Key"))
            {
                sb.Append($"{name}=***;");
            }
            else
            {
                sb.Append($"{name}={val};");
            }
        }
    }

    public override string ToString()
    {
        var result = new StringBuilder();
        AppendIfNotEmpty(result, nameof(HostName), HostName!);
        AppendIfNotEmpty(result, nameof(TcpPort), TcpPort.ToString());
        AppendIfNotEmpty(result, nameof(UserName), UserName!);
        AppendIfNotEmpty(result, nameof(X509PemPath), X509PemPath!);
        AppendIfNotEmpty(result, nameof(ClientId), ClientId!);
        AppendIfNotEmpty(result, nameof(Auth), Auth!.ToString());
        result.Remove(result.Length - 1, 1);
        return result.ToString();
    }
}
