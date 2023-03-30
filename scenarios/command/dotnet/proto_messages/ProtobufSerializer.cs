
using Google.Protobuf;

namespace proto_messages;

public class ProtobufSerializer : IMessageSerializer
{
    private readonly MessageParser? _parser;

    public ProtobufSerializer(MessageParser parser) => _parser = parser;

    public T FromBytes<T>(byte[] payload) => (T)_parser!.ParseFrom(payload);

    public byte[] ToBytes<T>(T payload) => (payload as IMessage).ToByteArray();
    
}
