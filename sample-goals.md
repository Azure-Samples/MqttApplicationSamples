# Sample Goals

- Show how to create MQTT applications with different programming languages, starting with C#, Python and C
- Each language will use a popular MQTT Library, MQTTNet for C# and Paho for Python and C
- Propose abstractions to encapsulate basic operations: Connect and Pub/Sub
- Create extension for each library to reuse common functions (aka Helpers) for repetitive tasks

## Getting Started Samples

- Connect to an existing MQTT Broker from your development environment.
- Use a console application, use the same connection to pub/sub to a sample topic.

### Configure Connection Settings 

- Have a broker already available, such as mosquitto, or Event Grid 
- Using VSCode `launch.json` to specify connection settings
- Sample settings: MQTTVersion, HostName, Port, KeepAlive, CleanSession, Credentials, ClientId and TLS with custom CA Trust.
     
#### Authenticate with Client Certificates

- Load certificates from Connection Settings as PEM/KEY files
- Validate certificates (expiration, trust chain)
- Parse cert SubjectName to be used as ClientId

#### Inspect Connection Results

- If the connection succeeds, show the values returned in the CONNACK: assigned clientId, enabled features, session present, auth data. 
- If the connection fails, show the error information

### Troubleshooting

- How to validate the connection with `mosquitto_sub` and `openssl`, map concepts to connections.


## Scenario 1. Telemetry

- Create two console applications to host long running processes:
  - Producer. To publish telemetry messages
  - Consumer. To subscribe to telemetry messages
  - Message Payload. Use GeoJSON to indicate `lat/lon`

- Connect to the broker using MQTT 3.1
- Create JSON payloads with _fake_ data
- Publish messages in a loop, with 5s delay
- Subscribe to the same topic, expose received messages in a callback function

Producer

```c
connAck = mqtt.connect(/* connection settings */);
telemetry = telemetry(mqtt, "sample/topic", &geo_json_serializer);
pub_ack = telemetry.send(lat, lon);
```

Consumer

```c
void on_message_received(char* client_id, int lat, int lon) {
    print(client_id, lat, lon);
}
conn_ack = mqtt.connect(/* connection settings */);
if (conn_ack.result == conn_result_codes.OK) {
    telemetry_consumer = telemetry_consumer(mqtt, &geo_json_serializer, &on_message_received);
    telemetry_consumer.start_monitoring("sample/+/topic");
}
```
