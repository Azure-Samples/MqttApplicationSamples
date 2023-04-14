# Telemetry (Fan-in)

## Create Client Certificates

vehicle01

```bash

step certificate create vehicle01 vehicle01.pem vehicle01.key --ca ~/.step/certs/intermediate_ca.crt --ca-key ~/.step/secrets/intermediate_ca_key --no-password --insecure --not-after 2400h
```

map-app

```
step certificate create map-app map-app.pem map-app.key --ca ~/.step/certs/intermediate_ca.crt --ca-key ~/.step/secrets/intermediate_ca_key --no-password --insecure --not-after 2400h
```

## Register Clients

```bash
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