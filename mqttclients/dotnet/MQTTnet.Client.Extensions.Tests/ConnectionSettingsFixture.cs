namespace MQTTnet.Extensions.MultiCloud.UnitTests;

public class ConnectionSettingsFixture
{
    [Fact]
    public void DefaultValues()
    {
        var dcs = new ConnectionSettings();
        Assert.Equal(60, dcs.KeepAliveInSeconds);
        Assert.Equal(AuthType.Basic, dcs.Auth);
        Assert.Equal(8883, dcs.TcpPort);
        Assert.False(dcs.DisableCrl);
        Assert.True(dcs.UseTls);
        Assert.Equal("TcpPort=8883;CleanSession=True;KeepAliveInSeconds=60;UseTls=True;Auth=Basic", dcs.ToString());
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
        ConnectionSettings dcs = new(cs);
        Assert.Equal("<hostname>", dcs.HostName);
        Assert.Equal(60, dcs.KeepAliveInSeconds);
    }


    [Fact]
    public void ParseConnectionStringWithDefaultValues()
    {
        string cs = "HostName=<hubname>.azure-devices.net";
        ConnectionSettings dcs = ConnectionSettings.FromConnectionString(cs);
        Assert.Equal("<hubname>.azure-devices.net", dcs.HostName);
        Assert.Equal(60, dcs.KeepAliveInSeconds);
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
        ConnectionSettings dcs = new()
        {
            HostName = "h",
        };
        string expected = "HostName=h;TcpPort=8883;CleanSession=True;KeepAliveInSeconds=60;UseTls=True;Auth=Basic";
        Assert.Equal(expected, dcs.ToString());
    }
}
