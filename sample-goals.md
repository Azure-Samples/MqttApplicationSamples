# Sample Goals

- Show how to create MQTT applications with different programming languages, starting with C#, Python and C
- Each language will use a popular MQTT Library, MQTTNet for C# and Paho for Python and C
- Propose abstractions to encapsulate basic operations: Connect and Pub/Sub

## Getting Started Samples

Connect to an existing MQTT Broker from your development environment

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
