# Telemetry (Fan-in)

| [Create the Client Certificates](#create-client-certificates) | [Configure Event Grid Namespaces](#configure-event-grid-namespaces) | [Configure mosquitto](#configure-mosquitto) |

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

```bash
source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"
hostname=$(az resource show --ids $res_id --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "HOST_NAME=$hostname" > vehicle01.env
echo "USERNAME=vehicle01" >> vehicle01.env
echo "CERT_FILE=vehicle01.pem" >> vehicle01.env
echo "KEY_FILE=vehicle01.key" >> vehicle01.env


echo "HOST_NAME=$hostname" > vehicle02.env
echo "USERNAME=vehicle02" >> vehicle02.env
echo "CERT_FILE=vehicle02.pem" >> vehicle02.env
echo "KEY_FILE=vehicle02.key" >> vehicle02.env

echo "HOST_NAME=$hostname" > map-app.env
echo "USERNAME=map-app" >> map-app.env
echo "CERT_FILE=map-app.pem" >> map-app.env
echo "KEY_FILE=map-app.key" >> map-app.env
```

## Configure Mosquitto 

To establish the TLS connection, the CA needs to be trusted, most MQTT clients allow to specify the ca trust chain as part of the connection, to create a chain file with the root and the intermediate use:

```bash
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
```
The `chain.pem` is used by mosquitto via the `cafile` settings to authenticate X509 client connections.

```bash
echo "HOST_NAME=localhost" > vehicle01.env
echo "CERT_FILE=vehicle01.pem" >> vehicle01.env
echo "KEY_FILE=vehicle01.key" >> vehicle01.env
echo "CA_FILE=chain.pem" >> vehicle01.env

echo "HOST_NAME=localhost" > vehicle02.env
echo "CERT_FILE=vehicle02.pem" >> vehicle02.env
echo "KEY_FILE=vehicle02.key" >> vehicle02.env
echo "CA_FILE=chain.pem" >> vehicle02.env

echo "HOST_NAME=localhost" > map-app.env
echo "CERT_FILE=map-app.pem" >> map-app.env
echo "KEY_FILE=map-app.key" >> map-app.env
echo "CA_FILE=chain.pem" >> map-app.env

```

To use mosquitto without certificates: change the port to 1883, disable TLS and set the CA_FILE

```bash
echo "HOST_NAME=localhost" > vehicle01.env
echo "TCP_PORT=1883" >> vehicle01.env
echo "USE_TLS=false" >> vehicle01.env
echo "CLIENT_ID=vehicle01" >> vehicle01.env
```