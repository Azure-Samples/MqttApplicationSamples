# :point_right: Alert (Fan-out)

| [Create Client Certificates](#lock-create-client-certificates) | [Configure Event Grid Namespaces](#triangular_ruler-configure-event-grid-namespaces) | [Configure Mosquitto](#fly-configure-mosquitto) | [Run the Sample](#game_die-run-the-sample) |

This scenario shows how to send a weather alert to a vehicle fleet. Instead of sending one message to each vehicle, the message is sent to a topic that is subscribed by all vehicles. This is a fan-out scenario. Additionally, to avoid missing any message in case of losing network connectivity, the vehicles are using a persistent session, this way the broker will keep the messages until the vehicle is connected again. The sender uses the `MessageExpiryInterval` to make the alert available for 5 minutes. 


|Client|Role|Operation|Topic/Topic Filter|
|------|----|---------|------------------|
|control-tower|sender|pub|vehicles/weather/alert|
|vehicle04|receiver|sub|vehicles/weather/alert|
|vehicle05|receiver|sub|vehicles/weather/alert|

The message will send the following payload:

```json
{
	"type": "Weather",
	"alert": "Heavy rain",
	"time": "2023-06-23T10:34:00Z"
}
```

## :lock: Create Client Certificates

Run the following step commands to create the client certificates for `vehicle04`, `vehicle05` and `control-tower` clients.

```bash
# from folder scenarios/alert
step certificate create \
    vehicle04 vehicle04.pem vehicle04.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

step certificate create \
    vehicle05 vehicle05.pem vehicle05.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

step certificate create \
    control-tower control-tower.pem control-tower.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

```

## :triangular_ruler: Configure Event Grid Namespaces

Event Grid Namespaces requires to register the clients, and the topic spaces to set the client permissions.

### Create the clients

The clients will be created based on the certificate subject, you can register the 3 clients in the portal or by running the script below:

```bash
# from folder scenarios/alert
source ../../az.env

az resource create --id "$res_id/clients/vehicle04" --properties '{
    "authenticationName": "vehicle04",
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "validationScheme": "SubjectMatchesAuthenticationName"
    },
    "attributes": {
            "type": "vehicle"
    },
    "description": "This is a test publisher client"
}'

az resource create --id "$res_id/clients/vehicle05" --properties '{
    "authenticationName": "vehicle05",
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "validationScheme": "SubjectMatchesAuthenticationName"
    },
    "attributes": {
            "type": "vehicle"
    },
    "description": "This is a test publisher client"
}'

az resource create --id "$res_id/clients/control-tower" --properties '{
    "authenticationName": "control-tower",
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "validationScheme": "SubjectMatchesAuthenticationName"
    },
    "attributes": {
            "type": "map-client"
    },
    "description": "This is a test subscriber client"
}'

```

### Configure topic spaces and permission bindings

```bash
# from folder scenarios/alert
az resource create --id "$res_id/topicSpaces/vehicles" --properties '{
    "topicTemplates": ["vehicles/#"]
}'

az resource create --id "$res_id/permissionBindings/vehiclesPub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"vehicles",
    "permission":"Publisher"
}'

az resource create --id "$res_id/permissionBindings/vehiclesSub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"vehicles",
    "permission":"Subscriber"
}'
```

### Create the .env files with connection details

The required `.env` files can be configured manually, we provide the script below as a reference to create those files, as they are ignored from git.

```bash
# from folder scenarios/alert
source ../../az.env
host_name=$(az resource show --ids $res_id --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "MQTT_HOST_NAME=$host_name" > vehicle04.env
echo "MQTT_USERNAME=vehicle04" >> vehicle04.env
echo "MQTT_CLIENT_ID=vehicle04" >> vehicle04.env
echo "MQTT_CERT_FILE=vehicle04.pem" >> vehicle04.env
echo "MQTT_KEY_FILE=vehicle04.key" >> vehicle04.env

echo "MQTT_HOST_NAME=$host_name" > vehicle05.env
echo "MQTT_USERNAME=vehicle05" >> vehicle05.env
echo "MQTT_CLIENT_ID=vehicle05" >> vehicle05.env
echo "MQTT_CERT_FILE=vehicle05.pem" >> vehicle05.env
echo "MQTT_KEY_FILE=vehicle05.key" >> vehicle05.env

echo "MQTT_HOST_NAME=$host_name" > control-tower.env
echo "MQTT_USERNAME=control-tower" >> control-tower.env
echo "MQTT_CLIENT_ID=control-tower" >> control-tower.env
echo "MQTT_CERT_FILE=control-tower.pem" >> control-tower.env
echo "MQTT_KEY_FILE=control-tower.key" >> control-tower.env
```

## :fly: Configure Mosquitto

To establish the TLS connection, the CA needs to be trusted, most MQTT clients allow to specify the ca trust chain as part of the connection, to create a chain file with the root and the intermediate use:

```bash
# from folder _mosquitto
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
cp chain.pem ../scenarios/alert
```
The `chain.pem` is used by mosquitto via the `cafile` settings to authenticate X509 client connections.

```bash
# from folder scenarios/alert
echo "MQTT_HOST_NAME=localhost" > vehicle04.env
echo "MQTT_CLIENT_ID=vehicle04" >> vehicle04.env
echo "MQTT_CERT_FILE=vehicle04.pem" >> vehicle04.env
echo "MQTT_KEY_FILE=vehicle04.key" >> vehicle04.env
echo "MQTT_CA_FILE=chain.pem" >> vehicle04.env

echo "MQTT_HOST_NAME=localhost" > vehicle05.env
echo "MQTT_CLIENT_ID=vehicle05" >> vehicle05.env
echo "MQTT_CERT_FILE=vehicle05.pem" >> vehicle05.env
echo "MQTT_KEY_FILE=vehicle05.key" >> vehicle05.env
echo "MQTT_CA_FILE=chain.pem" >> vehicle05.env

echo "MQTT_HOST_NAME=localhost" > control-tower.env
echo "MQTT_CLIENT_ID=control-tower" >> control-tower.env
echo "MQTT_CERT_FILE=control-tower.pem" >> control-tower.env
echo "MQTT_KEY_FILE=control-tower.key" >> control-tower.env
echo "MQTT_CA_FILE=chain.pem" >> control-tower.env

```

To use mosquitto without certificates: change the port to 1883 and disable TLS

```bash
# from folder scenarios/alert
echo "MQTT_HOST_NAME=localhost" > vehicle04.env
echo "MQTT_CLIENT_ID=vehicle04" >> vehicle04.env
echo "MQTT_TCP_PORT=1883" >> vehicle04.env
echo "MQTT_USE_TLS=false" >> vehicle04.env

echo "MQTT_HOST_NAME=localhost" > vehicle05.env
echo "MQTT_CLIENT_ID=vehicle05" >> vehicle05.env
echo "MQTT_TCP_PORT=1883" >> vehicle05.env
echo "MQTT_USE_TLS=false" >> vehicle05.env

echo "MQTT_HOST_NAME=localhost" > control-tower.env
echo "MQTT_CLIENT_ID=control-tower" >> control-tower.env
echo "MQTT_TCP_PORT=1883" >> control-tower.env
echo "MQTT_USE_TLS=false" >> control-tower.env
```

## :game_die: Run the Sample

All samples are designed to be executed from the root scenario folder.

### dotnet

To build the dotnet sample run:

```bash
# from folder scenarios/alert
dotnet build dotnet/alert.sln 
```

To run the dotnet sample execute each line below in a different shell/terminal.

```bash
# from folder scenarios/alert
 dotnet/vehicle/bin/Debug/net7.0/vehicle --envFile=vehicle04.env
```
```bash
 dotnet/vehicle/bin/Debug/net7.0/vehicle --envFile=vehicle05.env
```
```bash
 dotnet/control-tower/bin/Debug/net7.0/control-tower --envFile=control-tower.env
```

### C

TBD


### python

TBD