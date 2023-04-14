namespace MQTTnet.Client.Extensions;

public interface IMessageSerializer
{
    string ContentType { get; }
    byte[] ToBytes<T>(T payload);
    T FromBytes<T>(byte[] payload);
}
