# Command


vehicle01

```bash
step certificate create vehicle02 vehicle02.pem vehicle02.key --ca ~/.step/certs/intermediate_ca.crt --ca-key ~/.step/secrets/intermediate_ca_key --no-password --insecure --not-after 2400h
```

mobile-app
```bash
step certificate create mobile-app mobile-app.pem mobile-app.key --ca ~/.step/certs/intermediate_ca.crt --ca-key ~/.step/secrets/intermediate_ca_key --no-password --insecure --not-after 2400h
```

## Register Clients

```bash
source /../../.env
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

az resource create --id "$resid/clients/mobile-app" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "mobile-app"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'
```