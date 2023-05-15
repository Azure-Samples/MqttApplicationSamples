# Telemetry (Fan-in)

| [Create the Client Certificates](#create-client-certificates) | [Configure Event Grid Namespaces](#configure-event-grid-namespaces) | [Configure Mosquitto](#configure-mosquitto) | [Run the Sample](#run-the-sample) |

This scenario shows how multiple clients send data (the producers) to a different set of topics that can be consumed by a single application (the consumer).

Consider a use case where a backend solution needs to identify the location of vehicles on a map. Vehicles should be prohibited from listening to other vehicles location on their behalf.

|Client|Role|Operation|Topic/Topic Filter|
|------|----|---------|------------------|
|vehicle1|producer|pub|vehicles/vehicle1/position|
|vehicle2|producer|pub|vehicles/vehicle2/position|
|map-app|consumer|sub|vehicles/+/position|

Messages will use [GeoJSON](https://geojson.org) to represent the coordinates.

```json
{
    "type": "Point",
    "coordinates": [125.6, 10.1]
}
```


## Create Client Certificates

Run the following step commands to create the client certificates for `vehicle01`, `vehicle02` and `map-app`.

```bash
step certificate create \
    vehicle01 vehicle01.pem vehicle01.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

step certificate create \
    vehicle02 vehicle02.pem vehicle02.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

step certificate create \
    map-app map-app.pem map-app.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

```

## Configure Event Grid Namespaces

Event Grid Namespaces requires to register the clients, and the topic spaces to set the client permissions. 

### Create the Clients

The clients will be created based on the certificate subject, you can register the 3 clients in the portal or by running the script below:

```bash
source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"

az resource create --id "$res_id/clients/vehicle01" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "vehicle01"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'

az resource create --id "$res_id/clients/vehicle02" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "vehicle02"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'

az resource create --id "$res_id/clients/map-app" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "map-app"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'

```

### Configure Permissions with Topic Spaces

```bash
az resource create --id "$res_id/topicSpaces/vehicles" --properties '{
    "topicTemplates": ["vehicles/#"],
    "subscriptionSupport": "LowFanout"
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
source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"
host_name=$(az resource show --ids $res_id --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "MQTT_HOST_NAME=$host_name" > vehicle01.env
echo "MQTT_USERNAME=vehicle01" >> vehicle01.env
echo "MQTT_CERT_FILE=vehicle01.pem" >> vehicle01.env
echo "MQTT_KEY_FILE=vehicle01.key" >> vehicle01.env


echo "MQTT_HOST_NAME=$host_name" > vehicle02.env
echo "MQTT_USERNAME=vehicle02" >> vehicle02.env
echo "MQTT_CERT_FILE=vehicle02.pem" >> vehicle02.env
echo "MQTT_KEY_FILE=vehicle02.key" >> vehicle02.env

echo "MQTT_HOST_NAME=$host_name" > map-app.env
echo "MQTT_USERNAME=map-app" >> map-app.env
echo "MQTT_CERT_FILE=map-app.pem" >> map-app.env
echo "MQTT_KEY_FILE=map-app.key" >> map-app.env
```

## Configure Mosquitto 

To establish the TLS connection, the CA needs to be trusted, most MQTT clients allow to specify the ca trust chain as part of the connection, to create a chain file with the root and the intermediate use:

```bash
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
```
The `chain.pem` is used by mosquitto via the `cafile` settings to authenticate X509 client connections.

```bash
echo "MQTT_HOST_NAME=localhost" > vehicle01.env
echo "MQTT_CERT_FILE=vehicle01.pem" >> vehicle01.env
echo "MQTT_KEY_FILE=vehicle01.key" >> vehicle01.env
echo "MQTT_CA_FILE=chain.pem" >> vehicle01.env

echo "MQTT_HOST_NAME=localhost" > vehicle02.env
echo "MQTT_CERT_FILE=vehicle02.pem" >> vehicle02.env
echo "MQTT_KEY_FILE=vehicle02.key" >> vehicle02.env
echo "MQTT_CA_FILE=chain.pem" >> vehicle02.env

echo "MQTT_HOST_NAME=localhost" > map-app.env
echo "MQTT_CERT_FILE=map-app.pem" >> map-app.env
echo "MQTT_KEY_FILE=map-app.key" >> map-app.env
echo "MQTT_CA_FILE=chain.pem" >> map-app.env

```

To use mosquitto without certificates: change the port to 1883, disable TLS and set the CA_FILE

```bash
echo "MQTT_HOST_NAME=localhost" > vehicle01.env
echo "MQTT_TCP_PORT=1883" >> vehicle01.env
echo "MQTT_USE_TLS=false" >> vehicle01.env
echo "MQTT_CLIENT_ID=vehicle01" >> vehicle01.env
```

## Run the Sample

All samples are designed to be executed from the root scenario folder.

### dotnet

To build the dotnet sample run:

```bash
dotnet build dotnet/telemetry.sln 
```

To run the dotnet sample execute each line below in a different shell/terminal.

```bash
 dotnet/telemetry_producer/bin/Debug/net7.0/telemetry_producer --envFile=vehicle01.env
 dotnet/telemetry_producer/bin/Debug/net7.0/telemetry_producer --envFile=vehicle02.env
 dotnet/telemetry_consumer/bin/Debug/net7.0/telemetry_consumer --envFile=map-app.env
```

### C

To build the C sample run:

```bash
c/build.sh
```
The build script will copy the produced binary to `c/build/telemetry`

To run the C sample execute each line below in a different shell/terminal.

```
c/build/telemetry_producer vehicle01.env
c/build/telemetry_producer vehicle02.env
c/build/telemetry_consumer map-app.env

