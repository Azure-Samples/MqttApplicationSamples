namespace MQTTnet.Client.Extensions;

public class TelemetryMessage<T>
{
    public ushort MessageId { get; set; }
    public string? ClientIdFromTopic { get; set; }
    public T? Payload { get; set; }
}
