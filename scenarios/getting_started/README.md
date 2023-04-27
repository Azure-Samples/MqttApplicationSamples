# Getting Started

This sample shows how to perform basic MQTT operations: Connect, Publish and Subscribe.

| [Create the Client Certificate](#create-the-client-certificate) | [Configure Event Grid Namespaces](#configure-event-grid-namespaces) | [Configure mosquitto](#configure-mosquitto) |

## Create the client certificate

Using the CA files, as described in [setup](../setup), create a certificate for `vehicle01`.

```bash
cd scenarios/getting_started
step certificate create \
    vehicle01 vehicle01.pem vehicle01.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h
```

## Configure Event Grid Namespaces

Event Grid Namespaces requires to register the client, and the topic spaces to set the client permissions. 

### Create the Client

We will use the certificateSubject `vehicle01`, from the portal or with the script below:

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
```

### Configure Permissions with Topic Spaces

```bash
source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"

az resource create --id "$res_id/topicSpaces/samples" --properties '{
    "topicTemplates": ["sample/#"],
    "subscriptionSupport": "LowFanout"
}'

az resource create --id "$res_id/permissionBindings/samplesPub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"samples",
    "permission":"Publisher"
}'

az resource create --id "$res_id/permissionBindings/samplesSub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"samples",
    "permission":"Subscriber"
}'
```

### Create the .env file with connection details

```bash
cd scenarios/getting_started
source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"
hostname=$(az resource show --ids $res_id --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "HOST_NAME=$hostname" > .env
echo "USERNAME=vehicle01" >> .env
echo "CERT_FILE=vehicle01.pem" >> .env
echo "KEY_FILE=vehicle01.key" >> .env
```

## Configure Mosquitto 

To establish the TLS connection, the CA needs to be trusted, most MQTT clients allow to specify the ca trust chain as part of the connection, to create a chain file with the root and the intermediate use:

```bash
cd _mosquitto
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
```
The `chain.pem` is used by mosquitto via the `cafile` settings to authenticate X509 client connections.

```bash
echo "HOST_NAME=localhost" > .env
echo "CERT_FILE=vehicle01.pem" >> .env
echo "KEY_FILE=vehicle01.key" >> .env
echo "CA_FILE=chain.pem" >> .env
```

To use mosquitto without certificates

```bash
echo "HOST_NAME=localhost" > .env
echo "TCP_PORT=1883" >> .env
echo "USE_TLS=false" >> .env
echo "CLIENT_ID=vehicle01" >> .env
```