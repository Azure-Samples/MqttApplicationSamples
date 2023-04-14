# Telemetry (Fan-in)

## Create Client Certificates

vehicle01

```bash

step certificate create \
    vehicle01 vehicle01.pem vehicle01.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h



az resource create --id "$resid/clients/vehicle01" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "vehicle01"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'

source ../../az.env
resid="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"
hostname=$(az resource show --ids $resid --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "HOST_NAME=$hostname" > vehicle01.env
echo "USERNAME=vehicle01" >> vehicle01.env
echo "CERT_FILE=vehicle01.pem" >> vehicle01.env
echo "KEY_FILE=vehicle01.key" >> vehicle01.env

```

vehicle02

```bash
step certificate create \
    vehicle02 vehicle02.pem vehicle02.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

az resource create --id "$resid/clients/vehicle02" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "vehicle02"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'

echo "HOST_NAME=$hostname" > vehicle02.env
echo "USERNAME=vehicle02" >> vehicle02.env
echo "CERT_FILE=vehicle02.pem" >> vehicle02.env
echo "KEY_FILE=vehicle02.key" >> vehicle02.env
```

map-app

```bash
step certificate create \
    map-app map-app.pem map-app.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

az resource create --id "$resid/clients/map-app" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "map-app"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'

echo "HOST_NAME=$hostname" > map-app.env
echo "USERNAME=map-app" >> map-app.env
echo "CERT_FILE=map-app.pem" >> map-app.env
echo "KEY_FILE=map-app.key" >> map-app.env
```

## Configure Topics in EventGrid Namespaces

```bash
az resource create --id "$resid/topicSpaces/vehicles" --properties '{
    "topicTemplates": ["vehicles/#"],
    "subscriptionSupport": "LowFanout"
}'s

az resource create --id "$resid/permissionBindings/vehiclesPub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"vehicles",
    "permission":"Publisher"
}'

az resource create --id "$resid/permissionBindings/vehiclesSub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"vehicles",
    "permission":"Subscriber"
}'
```