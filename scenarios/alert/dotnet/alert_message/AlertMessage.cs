using System.Data;
using System.Text.Json.Serialization;

namespace alert_message
{
    public enum AlertType
    {
        Weather,
        Traffic,
        Accident
    }

    public class AlertMessage
    {
        [JsonPropertyName("type")]
        public AlertType AlertType { get; set; }
        [JsonPropertyName("alert")]
        public string? AlertText { get; set; }
        [JsonPropertyName("time")]
        public DateTime? AlertTime { get; set; }
    }

}