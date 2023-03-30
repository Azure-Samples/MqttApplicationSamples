namespace telemetry_consumer;

internal class TelemetryMessage<T>
{
    public string? ClientIdFromTopic { get; set; }
    public T? Payload { get; set; }
}
