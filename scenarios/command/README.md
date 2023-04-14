# Command

## Configure the clients

vehicle03

```bash
step certificate create \
    vehicle03 vehicle03.pem vehicle03.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"

az resource create --id "$res_id/clients/vehicle03" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "vehicle03"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'

hostname=$(az resource show --ids $res_id --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "HOST_NAME=$hostname" > vehicle03.env
echo "USERNAME=vehicle03" >> vehicle03.env
echo "CERT_FILE=vehicle03.pem" >> vehicle03.env
echo "KEY_FILE=vehicle03.key" >> vehicle03.env
```

mobile-app
```bash
step certificate create \
    mobile-app mobile-app.pem mobile-app.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"
hostname=$(az resource show --ids $res_id --query "properties.topicSpacesConfiguration.hostname" -o tsv)

az resource create --id "$res_id/clients/mobile-app" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "mobile-app"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'

echo "HOST_NAME=$hostname" > mobile-app.env
echo "USERNAME=mobile-app" >> mobile-app.env
echo "CERT_FILE=mobile-app.pem" >> mobile-app.env
echo "KEY_FILE=mobile-app.key" >> mobile-app.env
```
## Configure Topics in EventGrid Namespaces

```bash
az resource create --id "$res_id/topicSpaces/vehiclesCommands" --properties '{
    "topicTemplates": ["vehicles/+/command/#"],
    "subscriptionSupport": "LowFanout"
}'

az resource create --id "$res_id/permissionBindings/vehiclesPub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"vehiclesCommands",
    "permission":"Publisher"
}'

az resource create --id "$res_id/permissionBindings/vehiclesSub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"vehiclesCommands",
    "permission":"Subscriber"
}'
```