using System.Text.Json;

namespace MQTTnet.Client.Extensions;
public class Utf8JsonSerializer : IMessageSerializer
{
    public string ContentType => "application/json";

    public T FromBytes<T>(byte[] payload)
    {
        Utf8JsonReader reader = new Utf8JsonReader(payload);
        return JsonSerializer.Deserialize<T>(ref reader)!;
    }

    public byte[] ToBytes<T>(T payload) => JsonSerializer.SerializeToUtf8Bytes(payload);
}
