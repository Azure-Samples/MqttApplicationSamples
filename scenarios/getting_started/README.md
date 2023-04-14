# Getting Started Samples

This sample shows how to perform basic MQTT operations: Connect, Publish and Subscribe.

The connection requires client certificates issued by a known CA.


## Locate CA cert files

Generate a CA for this samples as described in [setup](../setup), by default the root and intermediate certificates are stored in:

- `~/.certs/root_ca.crt`
- `~/.certs/intermediate_ca.crt`

## Configure the client

To configure the client you need to  generate a client certificate, register the client, and create the .env file with those settings:

```bash
cd getting_started
step certificate create \
    vehicle01 vehicle01.pem vehicle01.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

source ../../az.env
resid="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"

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


hostname=$(az resource show --ids $resid --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "HOST_NAME=$hostname" > .env
echo "USERNAME=vehicle01" >> .env
echo "CERT_FILE=vehicle01.pem" >> .env
echo "KEY_FILE=vehicle01.key" >> .env
```


## Configure EG

```bash
source ../../az.env
resid="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"

az resource create --id "$resid/topicSpaces/samples" --properties '{
    "topicTemplates": ["sample/#"],
    "subscriptionSupport": "LowFanout"
}'

az resource create --id "$resid/permissionBindings/samplesPub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"samples",
    "permission":"Publisher"
}'

az resource create --id "$resid/permissionBindings/samplesSub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"samples",
    "permission":"Subscriber"
}'
```

## Configure mosquitto to accept TLS certs

To establish the TLS connection, the CA needs to be trusted, most MQTT clients allow to specify the ca trust chain as part of the connection, to create a chain file with the root and the intermediate use:

```bash
cd _mosquitto
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
```
The `chain.pem` is used by mosquitto via the `cafile` settings to authenticate X509 client connections.
