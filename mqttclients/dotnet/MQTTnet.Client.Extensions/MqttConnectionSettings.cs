using System.Diagnostics;
using System.Text;

namespace MQTTnet.Client.Extensions;

public enum AuthType
{
    X509,
    Basic
}

public class MqttConnectionSettings
{
    private const int Default_KeepAliveInSeconds = 30;
    private const string Default_CleanSession = "true";
    private const int Default_TcpPort = 8883;
    private const string Default_UseTls = "true";
    private const string Default_DisableCrl = "false";

    public string HostName { get; }
    public string? ClientId { get; set; }
    public string? CertFile { get; set; }
    public string? KeyFile { get; set; }
    public string? KeyFilePassword { get; set; }

    public AuthType Auth
    {
        get => !string.IsNullOrEmpty(CertFile) ? AuthType.X509 : AuthType.Basic;
    }
    public string? Username { get; set; }
    public string? Password { get; set; }
    public int KeepAliveInSeconds { get; set; }
    public bool CleanSession { get; set; }
    public int TcpPort { get; set; }
    public bool UseTls { get; set; }
    public string? CaFile { get; set; }
    public bool DisableCrl { get; set; }


    public MqttConnectionSettings(string hostname)
    {
        HostName = hostname;
        TcpPort = Default_TcpPort;
        KeepAliveInSeconds = Default_KeepAliveInSeconds;
        UseTls = Default_UseTls == "true";
        DisableCrl = Default_DisableCrl == "true";
        CleanSession = Default_CleanSession == "true";
    }

    public static MqttConnectionSettings FromConnectionString(string cs) => ParseConnectionString(cs);

    public static MqttConnectionSettings CreateFromEnvVars(string? envFile = "")
    {
        if (string.IsNullOrEmpty(envFile))
        {
            envFile = ".env";
        }

        if (File.Exists(envFile))
        {
            Trace.TraceInformation("Loading environment variables from {envFile}" + new FileInfo(envFile).FullName);
            foreach (string line in File.ReadAllLines(envFile))
            {
                string[] parts = line.Split('=', StringSplitOptions.RemoveEmptyEntries);
                if (parts.Length != 2)
                {
                    continue;
                }

                Environment.SetEnvironmentVariable(parts[0], parts[1]);
            }
        }
        else
        {
            Trace.TraceWarning($"EnvFile Not found in path {new DirectoryInfo(".").FullName} {envFile}");
        }
        static string ToUpperCaseFromPascalCase(string pascal) =>
            string.Concat(pascal.Select(x => Char.IsUpper(x) ? "_" + x : x.ToString())).ToUpper().TrimStart('_');

        static string Env(string name) =>
            Environment.GetEnvironmentVariable("MQTT_" + ToUpperCaseFromPascalCase(name)) ?? string.Empty;

        string hostname = Env(nameof(HostName));

        ArgumentException.ThrowIfNullOrEmpty(hostname, nameof(hostname));

        return new MqttConnectionSettings(hostname)
        {
            ClientId = Env(nameof(ClientId)),
            CertFile = Env(nameof(CertFile)),
            KeyFile = Env(nameof(KeyFile)),
            Username = Env(nameof(Username)),
            Password = Env(nameof(Password)),
            KeepAliveInSeconds = string.IsNullOrEmpty(Env(nameof(KeepAliveInSeconds))) ? Default_KeepAliveInSeconds : CheckForValidIntegerInput(nameof(KeepAliveInSeconds), Env(nameof(KeepAliveInSeconds))),
            CleanSession = Env(nameof(CleanSession)) == "true",
            TcpPort = string.IsNullOrEmpty(Env(nameof(TcpPort))) ? Default_TcpPort : CheckForValidIntegerInput(nameof(TcpPort), Env(nameof(TcpPort))),
            UseTls = string.IsNullOrEmpty(Env(nameof(UseTls))) ? true : CheckForValidBooleanInput(nameof(UseTls), Env(nameof(UseTls))),
            CaFile = Env(nameof(CaFile)),
            DisableCrl = Env(nameof(DisableCrl)) == "true",
            KeyFilePassword = Env(nameof(KeyFilePassword)),

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

    private static MqttConnectionSettings ParseConnectionString(string connectionString)
    {

        IDictionary<string, string> map = connectionString.ToDictionary(';', '=');
        string hostName = GetStringValue(map, nameof(HostName));
        ArgumentNullException.ThrowIfNull(hostName, nameof(hostName));

        MqttConnectionSettings cs = new(hostName)
        {
            ClientId = GetStringValue(map, nameof(ClientId)),
            KeyFile = GetStringValue(map, nameof(KeyFile)),
            CertFile = GetStringValue(map, nameof(CertFile)),
            Username = GetStringValue(map, nameof(Username)),
            Password = GetStringValue(map, nameof(Password)),
            KeepAliveInSeconds = GetPositiveIntValueOrDefault(map, nameof(KeepAliveInSeconds), Default_KeepAliveInSeconds),
            CleanSession = GetStringValue(map, nameof(CleanSession), Default_CleanSession) == "true",
            TcpPort = GetPositiveIntValueOrDefault(map, nameof(TcpPort), Default_TcpPort),
            UseTls = GetStringValue(map, nameof(UseTls), Default_UseTls) == "true",
            CaFile = GetStringValue(map, nameof(CaFile)),
            DisableCrl = GetStringValue(map, nameof(DisableCrl), Default_DisableCrl) == "true"
        };
        return cs;
    }

    private static int CheckForValidIntegerInput(string envVarName, string envVarValue)
    {
        if (int.TryParse(envVarValue, out var result))
        {
            return result;
        }

        throw new ArgumentException($"An environment variable's type was specified incorrectly: {envVarName}={envVarValue}. Expecting an integer value.");
    }

    private static bool CheckForValidBooleanInput(string envVarName, string envVarValue)
    {
        if (bool.TryParse(envVarValue, out var result))
        {
            return result;
        }

        throw new ArgumentException($"An environment variable's type was specified incorrectly: {envVarName}={envVarValue}. Expecting a boolean value.");
    }

    private static void AppendIfNotEmpty(StringBuilder sb, string name, string val)
    {
        if (!string.IsNullOrEmpty(val))
        {
            sb.Append($"{name}={val};");
        }
    }

    public override string ToString()
    {
        StringBuilder result = new ();
        AppendIfNotEmpty(result, nameof(HostName), HostName!);
        AppendIfNotEmpty(result, nameof(TcpPort), TcpPort.ToString());
        AppendIfNotEmpty(result, nameof(Username), Username!);
        AppendIfNotEmpty(result, nameof(CleanSession), CleanSession.ToString());
        AppendIfNotEmpty(result, nameof(KeepAliveInSeconds), KeepAliveInSeconds.ToString());
        AppendIfNotEmpty(result, nameof(CertFile), CertFile!);
        AppendIfNotEmpty(result, nameof(KeyFile), KeyFile!);
        AppendIfNotEmpty(result, nameof(CaFile), CaFile!);
        AppendIfNotEmpty(result, nameof(ClientId), ClientId!);
        AppendIfNotEmpty(result, nameof(UseTls), UseTls.ToString());
        AppendIfNotEmpty(result, nameof(Auth), Auth!.ToString());
        result.Remove(result.Length - 1, 1);
        return result.ToString();
    }
}

