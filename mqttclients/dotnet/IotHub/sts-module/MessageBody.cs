using System.Text.Json.Serialization;

namespace STSModule
{
    class MessageBody
    {
        [JsonPropertyName("machine")]
        public Machine? Machine { get; set; }

        [JsonPropertyName("ambient")]
        public Ambient? Ambient { get; set; }

        [JsonPropertyName("timeCreated")]
        public DateTime TimeCreated { get; set; }
    }

    class Machine
    {
        [JsonPropertyName("temperature")]
        public double Temperature { get; set; }

        [JsonPropertyName("pressure")]
        public double Pressure { get; set; }
    }

    class Ambient
    {
        [JsonPropertyName("temperature")]
        public double Temperature { get; set; }

        [JsonPropertyName("humidity")]
        public int Humidity { get; set; }
    }
}