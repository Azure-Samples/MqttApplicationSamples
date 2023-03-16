using System.Text;

namespace MQTTnet.Extensions.MultiCloud.Connections;

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
    public string? DeviceId { get; set; }
    public string? ClientId { get; set; }
    public string? X509Key { get; set; } //paht-to.pfx|pfxpwd, or thumbprint
    public AuthType Auth
    {
        get => !string.IsNullOrEmpty(X509Key) ? AuthType.X509 : AuthType.Basic;
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
        DeviceId = GetStringValue(map, nameof(DeviceId));
        ClientId = GetStringValue(map, nameof(ClientId));
        X509Key = GetStringValue(map, nameof(X509Key));
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
        AppendIfNotEmpty(result, nameof(DeviceId), DeviceId!);
        AppendIfNotEmpty(result, nameof(UserName), UserName!);
        AppendIfNotEmpty(result, nameof(X509Key), X509Key!);
        AppendIfNotEmpty(result, nameof(ClientId), ClientId!);
        AppendIfNotEmpty(result, nameof(Auth), Auth!.ToString());
        result.Remove(result.Length - 1, 1);
        return result.ToString();
    }
}
