namespace MQTTnet.Client.Extensions
{
    [AttributeUsage(AttributeTargets.Class)]
    public class ResponseTopicAttribute : Attribute
    {
        public string Topic { get; set; }
        public ResponseTopicAttribute(string topic)
        {
            Topic = topic;
        }
    }
}
