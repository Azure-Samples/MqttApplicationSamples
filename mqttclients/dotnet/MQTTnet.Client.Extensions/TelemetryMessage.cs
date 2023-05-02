namespace MQTTnet.Client.Extensions;

public class TelemetryMessage<T>
{
    public string? ClientIdFromTopic { get; set; }
    public T? Payload { get; set; }
}
