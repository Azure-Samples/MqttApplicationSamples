# Goals for MQTT Application Samples

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

- How to validate the connection with `mosquitto_sub` and `openssl`


## Scenario 1. Telemetry

- Create two console applications to host long running processes:
  - Producer. To publish telemetry messages
  - Consumer. To subscribe to telemetry messages
  - Message Payload. Use GeoJSON to indicate `lat/lon`

- Connect to the broker using MQTT 3.1.1
- Create JSON payloads with _fake_ data
- Publish messages in a loop, with 5s delay
- Subscribe to the same topic, expose received messages in a callback function

Producer

```c
conn_ack = mqtt.connect(/* connection settings */);
if (conn_ack.result == conn_result_codes.OK) {
    telemetry = telemetry(mqtt, "sample/topic", &geo_json_serializer);
    pub_ack = telemetry.send(lat, lon);
}
```

Consumer

```c
void on_message_received(char* client_id, int lat, int lon) {
    print(client_id, lat, lon);
}
conn_ack = mqtt.connect(/* connection settings */);
if (conn_ack.result == conn_result_codes.OK) {
    telemetry_consumer = telemetry_consumer(mqtt, &geo_json_serializer, &on_message_received);
    telemetry_consumer.start("sample/+/topic");
}
```
## Scenario 2. Commands

- Show how to implement the `Command` pattern (aka RPC) using the MQTT Protocol
- Use MQTT5 features: Correlation ID, and ResponseTopic
- Use Protobuf to show how to use binary payloads

- Implement two console applications to host long running operations
- CommandServer to encapsulate:
  - Sub to request topic
  - Deserialize the request
  - Invoke callback with the deserialized payload
  - Pub response to response topic with correlation

- CommandClient to Invoke the command
  - Pub to request topic with correlation
  - Serialize payload 
  - Wait for response with a timeout (waiting thread?)
  - Sub to response topic, validate correlation

  Command Server
  
```c
&response_payload request_callback(&request_payload) {
    //command implementation
    // process request
    return response;
}


conn_ack = mqtt.connect(/* connection settings */);
if (conn_ack.result == conn_result_codes.OK) {
    command = command_server(mqtt, char* request_topic, &proto_serializer);
    command.on_request(&request_callback);
}
```

Command Client

```c
conn_ack = mqtt.connect(/* connection settings */);
if (conn_ack.result == conn_result_codes.OK) {
    command_client = command_client(mqtt, &proto_serializer);
    response = command_client.invoke(char* client_id, request);
}
```