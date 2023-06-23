namespace MQTTnet.Client.Extensions
{
    [AttributeUsage(AttributeTargets.Class)]
    public class RequestTopicAttribute : Attribute
    {
        public string Topic { get; set; }
        public RequestTopicAttribute(string topic)
        {
            Topic = topic;
        }
    }
}