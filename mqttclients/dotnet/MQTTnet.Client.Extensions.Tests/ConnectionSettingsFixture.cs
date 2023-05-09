using Xunit.Sdk;

namespace MQTTnet.Extensions.MultiCloud.UnitTests;

public class ConnectionSettingsFixture
{
    [Fact]
    public void DefaultValues()
    {
        var dcs = new ConnectionSettings("localhost");
        Assert.Equal("localhost", dcs.HostName);
        Assert.Equal(30, dcs.KeepAliveInSeconds);
        Assert.Equal(AuthType.Basic, dcs.Auth);
        Assert.Equal(8883, dcs.TcpPort);
        Assert.False(dcs.DisableCrl);
        Assert.True(dcs.UseTls);
        Assert.Equal("HostName=localhost;TcpPort=8883;CleanSession=True;KeepAliveInSeconds=30;UseTls=True;Auth=Basic", dcs.ToString());
    }

    [Fact]
    public void ParseConnectionString()
    {
        string cs = "HostName=<hostname>;ClientId=<clientId>";
        ConnectionSettings dcs = ConnectionSettings.FromConnectionString(cs);
        Assert.Equal("<hostname>", dcs.HostName);
        Assert.Equal("<clientId>", dcs.ClientId);
    }

    [Fact]
    public void InvalidValuesDontUseDefaults()
    {
        string cs = "HostName=<hostname>;KeepAliveInSeconds=invalid_string";
        ConnectionSettings dcs = ConnectionSettings.FromConnectionString(cs);
        Assert.Equal("<hostname>", dcs.HostName);
        Assert.Equal(30, dcs.KeepAliveInSeconds);
    }


    [Fact]
    public void ParseConnectionStringWithDefaultValues()
    {
        string cs = "HostName=<hubname>.azure-devices.net";
        ConnectionSettings dcs = ConnectionSettings.FromConnectionString(cs);
        Assert.Equal("<hubname>.azure-devices.net", dcs.HostName);
        Assert.Equal(30, dcs.KeepAliveInSeconds);
        Assert.Equal(8883, dcs.TcpPort);
        Assert.Empty(dcs.ClientId!);
        Assert.True(dcs.UseTls);
        Assert.False(dcs.DisableCrl);
    }

    [Fact]
    public void ParseConnectionStringWithAllValues()
    {
        string cs = """
                     HostName=<hubname>.azure-devices.net;
                     ClientId=<ClientId>;
                     CertFile=<certFile>;
                     KeyFile=<keyFile>;
                     TcpPort=1234;
                     UseTls=false;
                     CaFile=<path>;
                     DisableCrl=true;
                     UserName=<usr>;
                     Password=<pwd>
                     """.ReplaceLineEndings(String.Empty);

        ConnectionSettings dcs = ConnectionSettings.FromConnectionString(cs);
        Assert.Equal("<hubname>.azure-devices.net", dcs.HostName);
        Assert.Equal("<ClientId>", dcs.ClientId);
        Assert.Equal("<certFile>", dcs.CertFile);
        Assert.Equal("<keyFile>", dcs.KeyFile);
        Assert.Equal(1234, dcs.TcpPort);
        Assert.False(dcs.UseTls);
        Assert.Equal("<path>", dcs.CaFile);
        Assert.True(dcs.DisableCrl);
        Assert.Equal("<usr>", dcs.UserName);
        Assert.Equal("<pwd>", dcs.Password);
    }

    [Fact]
    public void ToStringReturnConnectionString()
    {
        ConnectionSettings dcs = new("h");
        string expected = "HostName=h;TcpPort=8883;CleanSession=True;KeepAliveInSeconds=30;UseTls=True;Auth=Basic";
        Assert.Equal(expected, dcs.ToString());
    }

    [Fact]
    public void CreateFromEnvFile_WithAllSettings()
    {
        var cs = ConnectionSettings.CreateFromEnvVars("test.env");
        Assert.Equal("localhost", cs.HostName);
        Assert.Equal(2883, cs.TcpPort);
        Assert.False(cs.UseTls);
        Assert.False(cs.CleanSession);
        Assert.Equal(32, cs.KeepAliveInSeconds);
        Assert.Equal("sample_client", cs.ClientId);
        Assert.Equal("sample_user", cs.UserName);
        Assert.Equal("foo", cs.Password);
        Assert.Equal("ca.pem", cs.CaFile);
        Assert.Equal("cert.pem", cs.CertFile);
        Assert.Equal("cert.key", cs.KeyFile);
        Assert.Equal("bar", cs.KeyFilePassword);
        RemoveTestEnvVars("test.env");
    }

    [Fact]
    public void CreateFromEnvFile_Defaults()
    {
        var cs = ConnectionSettings.CreateFromEnvVars("min.env");
        Assert.Equal("localhost", cs.HostName);
        Assert.Equal(8883, cs.TcpPort);
        Assert.True(cs.UseTls);
        Assert.False(cs.CleanSession);
        Assert.Equal(30, cs.KeepAliveInSeconds);
        Assert.Empty(cs.ClientId!);
        Assert.Empty(cs.UserName!);
        Assert.Empty(cs.Password!);
        Assert.Empty(cs.CaFile!);
        Assert.Empty(cs.CertFile!);
        Assert.Empty(cs.KeyFile!);
        Assert.Empty(cs.KeyFilePassword!);
    }

    void RemoveTestEnvVars(string envFile)
    {
        foreach (var line in File.ReadAllLines(envFile))
        {
            var parts = line.Split('=', StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length != 2) continue;
            Environment.SetEnvironmentVariable(parts[0], null);
        }
    }
}
