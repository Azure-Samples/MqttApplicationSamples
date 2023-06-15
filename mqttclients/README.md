# Mqtt client extensions

## Connection Settings as environment variables

All samples in this repo can be configured by using the next environment variables.

|EnvVar Name|Required|Type|DefaultValue|Notes|
|-----------|--------|----|------------|-----|
|`MQTT_HOST_NAME`|yes|string|n/a|FQDN to the endpoint, eg: mybroker.mydomain.com|
|`MQTT_TCP_PORT`|no|int|8883|TCP port to access the endpoint eg: 8883|
|`MQTT_USE_TLS`|no|bool|true|Disable TLS negotiation (not recommended for production)|
|`MQTT_CLEAN_SESSION`|no|bool|true|MQTT Clean Session, might require to set the ClientId **[existing sessions are not supported now]**|
|`MQTT_KEEP_ALIVE_IN_SECONDS`|no|int|30(*)|Seconds to send the ping to keep the connection open|
|`MQTT_CLIENT_ID`|no|string|empty(**)|MQTT Client Id|
|`MQTT_USERNAME`|no|string|empty|MQTT Username to authenticate the connection|
|`MQTT_PASSWORD`|no|string|empty|MQTT Password to authenticate the connection|
|`MQTT_CA_FILE`|no|string|empty|Path to a PEM file with the chain required to trust the TLS endpoint certificate|
|`MQTT_CERT_FILE`|no|string|empty|Path to a PEM file to establish X509 client authentication|
|`MQTT_KEY_FILE`|no|string|empty|Path to a KEY file to establish X509 client authentication|
|`MQTT_KEY_FILE_PASSWORD`|no|string|empty|Password (aka pass-phrase) to protect the key file| 

> (*) May vary by MQTT Client

> (**) ClientID might be assigned for the server, and it's required for CleanSession=false

When a variable does not match the type, eg trying to set the port to a string value, we will generate an error.

The _env vars_ are using `UPPER_CASE` casing, each language will assign these values to variables following the language style, eg: in `C#` we will use `PascalCase` and `C` will use `snake_case`, so `HOST_NAME` will be assigned to `HostName` and `host_name`.

