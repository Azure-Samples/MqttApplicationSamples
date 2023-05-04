# Goals for MQTT Application Samples

- Show how to create MQTT applications with different programming languages, starting with C#, Python and C
- Each language will use a popular MQTT Library, MQTTNet for C# and Paho for Python and Mosquitto_lib for C
- Propose abstractions to encapsulate basic operations: Connect and Pub/Sub
- Create extension for each library to reuse common functions (aka Helpers) for repetitive tasks
- Have CI to validate samples, with unit tests for the library extensions, and integration tests to run the samples

# Align samples for all languages

- Common folder structure for all languages
- Reuse connection settings for samples using different languages

## Getting Started Samples

- Connect to an existing MQTT Broker (mosquitto, EventGrid Namespaces, ..) from your development environment.
- Use a console application with the same connection to pub/sub to a sample topic.

### Configure Connection Settings 

- Have a broker already available, such as mosquitto, or Azure EventGrid Namespace
- Configure connection settings using `.env` files
- Samples can run from the command line, and using VSCode `launch.json`
- Sample settings: MQTTVersion, HostName, Port, KeepAlive, CleanSession, Credentials, ClientId and TLS with custom CA Trust.

#### Connection Settings

To align connection settings across languages we will the next environment variables:

|EnvVar Name|Required|Type|DefaultValue|Notes|
|-----------|--------|----|------------|---------------|-----|
|`HOST_NAME`|yes|string|n/a|FQDN to the endpoint, eg: mybroker.mydomain.com|
|`TCP_PORT`|no|int|8883|`TCP port to access the endpoint eg: 8883|
|`USE_TLS`|no|bool|true|Disable TLS negotiation (not recommended for production)|
|`CLEAN_SESSION`|no|bool|true|MQTT Clean Session, might require to set the ClientId|
|`KEEP_ALIVE_IN_SECONDS`|no|int|30(*)|Seconds to send the ping to keep the connection open|
|`CLIENT_ID`|no|string|empty(**)|MQTT Client Id|
|`USER_NAME`|no|string|empty|MQTT Username to authenticate the connection|
|`PASSWORD`|no|string|empty|MQTT Password to authenticate the connection|
|`CA_FILE`|no|string|empty|Path to a PEM file with the chain required to trust the TLS endpoint certificate|
|`CA_PATH`|no|string|empty|Path to a folder with trusted certs, eg: `/etc/ssl/certs`|
|`CERT_FILE`|no|string|empty|Path to a PEM file to establish X509 client authentication|
|`KEY_FILE`|no|string|empty|Path to a KEY file to establish X509 client authentication|
|`KEY_FILE_PASSWORD`|no|string|empty|Password (aka pass-phrase) to protect the key file| 

> (*) May vary by MQTT Client

> (**) ClientID might be assigned for the server, and it's required for CleanSession=false.

When a variable does not match the type, eg trying to set the port to a string value, we will generate an error.

The _env vars_ are using `UPPER_CASE`, each language will assign these values to variables following the language style, eg: in `C#` we will use `PascalCase` and `C` will use `snake_case`, so `HOST_NAME` will be assigned to `HostName` and `host_name`.

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
- The producer publishes messages in a loop, with 5s delay
- The consumer subscribes to the same topic, expose received messages in a callback function

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