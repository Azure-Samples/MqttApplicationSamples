# Using dotnet samples

These samples are based on [MQTTnet](https://github.com/dotnet/mqttnet)

## MQTTnet extensions

To avoid duplicating the code in each scenario, the `MQTTnet.Client.Extensions` provide extension methods to configure MQTTnet from `.env` files:

- `MqttConnectionSettings` allows to load the settings from .env files
- `WithConnectionSettings` is an extension method to MqttClientOptionsBuilder to open a connection from the loaded settings. 

```c#
var cs = MqttConnectionSettings.CreateFromEnvVars();
var mqttClient = new MqttFactory().CreateMqttClient(MqttNetTraceLogger.CreateTraceLogger());
var connAck = await mqttClient!.ConnectAsync(new MqttClientOptionsBuilder().WithConnectionSettings(cs).Build());
```

### Loading EnvVars

The samples will try to load the known environment variables and additionally will set those variables from .env files following the next order

- Passing a `--envFile` to the samples will force to load those settings
- When `--envFile` is not set, the program will try to load a `.env` file in the `$cwd`
- In all cases, the program will try to load the variables from the EnvVars available in the shell process.


## Tracing

Note the `mqttClient` is initialized with a trace logger `MqttNetTraceLogger` that will show traces on the output debug string, to redirect these traces to the main console output configure a `ConsoleTraceListener`:

```c#
System.Diagnostics.Trace.Listeners.Add(new System.Diagnostics.ConsoleTraceListener());
```
