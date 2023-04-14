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

source ../../.env
resid="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"
hostname=$(az resource show --ids $resid --query "properties.topicSpacesConfiguration.hostname" -o tsv)

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

echo "HostName=$hostname" > vehicle01.env
echo "UserName=vehicle01" >> vehicle01.env
echo "X509PemPath=vehicle01.pem" >> vehicle01.env
echo "X509KeyPath=vehicle01.key" >> vehicle01.env

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

echo "HostName=$hostname" > vehicle02.env
echo "UserName=vehicle02" >> vehicle02.env
echo "X509PemPath=vehicle02.pem" >> vehicle02.env
echo "X509KeyPath=vehicle02.key" >> vehicle02.env
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

echo "HostName=$hostname" > map-app.env
echo "UserName=map-app" >> map-app.env
echo "X509PemPath=map-app.pem" >> map-app.env
echo "X509KeyPath=map-app.key" >> map-app.env
```




## Register Clients in Event Grid Namespace

```bash
source ../../.env
resid="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"

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
```

## Configure Topics

```bash
az resource create --id "$resid/topicSpaces/vehicles" --properties '{
    "topicTemplates": ["vehicles/#"],
    "subscriptionSupport": "LowFanout"
}'

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