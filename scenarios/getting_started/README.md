# :point_right: Getting Started

This sample shows how to perform basic MQTT operations: Connect, Publish and Subscribe.

| [Create the Client Certificate](#create-the-client-certificate) | [Configure Event Grid Namespaces](#configure-event-grid-namespaces) | [Configure Mosquitto](#configure-mosquitto) | [Run the Sample](#run-the-sample) |

- Connect with MQTT 3.1.1
  - Validate TLS certificate enforcing TLS 1.2
  - Authenticate with client certificates
  - Configure connection settings such as KeepAlive and CleanSession
- Publish messages to a topic
- Subscribe to a topic to receive messages


##  :lock: Create the client certificate

Using the CA created in [setup](../setup), issue a leaf certificate for `sample_client`.

```bash
cd scenarios/getting_started
step certificate create \
    sample_client sample_client.pem sample_client.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h
```

## :triangular_ruler: Configure Event Grid Namespaces

Event Grid Namespaces requires to register the client, and the topic spaces to set the client permissions. 

### Create the Client

We will use the certificateSubject `sample_client`, from the portal or with the script below:

```bash
source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"

az resource create --id "$res_id/clients/sample_client" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "sample_client"
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

The required `.env` files can be configured manually, we provide the script below as a reference to create those files, as they are ignored from git.

```bash
cd scenarios/getting_started
source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"
host_name=$(az resource show --ids $res_id --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "MQTT_HOST_NAME=$host_name" > .env
echo "MQTT_USERNAME=sample_client" >> .env
echo "MQTT_CLIENT_ID=sample_client" >> .env
echo "MQTT_CERT_FILE=sample_client.pem" >> .env
echo "MQTT_KEY_FILE=sample_client.key" >> .env
echo "MQTT_CA_PATH=/etc/ssl/certs" >> .env # required by mosquitto_lib to validate EG Tls cert 
```

## :fly: Configure Mosquitto 

To establish the TLS connection, the CA needs to be trusted, most MQTT clients allow to specify the ca trust chain as part of the connection, to create a chain file with the root and the intermediate use:

```bash
cd _mosquitto
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
```
The `chain.pem` is used by mosquitto via the `cafile` settings to authenticate X509 client connections.

```bash
echo "MQTT_HOST_NAME=localhost" > .env
echo "MQTT_CLIENT_ID=sample_client" >> .env
echo "MQTT_CERT_FILE=sample_client.pem" >> .env
echo "MQTT_KEY_FILE=sample_client.key" >> .env
echo "MQTT_CA_FILE=chain.pem" >> .env
```

To use mosquitto without certificates

```bash
echo "MQTT_HOST_NAME=localhost" > .env
echo "MQTT_TCP_PORT=1883" >> .env
echo "MQTT_USE_TLS=false" >> .env
echo "MQTT_CLIENT_ID=sample_client" >> .env
```

## :game_die: Run the Sample

All samples are designed to be executed from the root scenario folder.

### dotnet

To build the dotnet sample run:

```bash
dotnet build dotnet/getting_started.sln 
```

To run the dotnet sample:

```bash
 dotnet/getting_started/bin/Debug/net7.0/getting_started
```
(this will use the `.env` file created before)

### C

To build the C sample, run from the root folder:

```bash
cmake --preset=getting_started
cmake --build --preset=getting_started
```
The build script will copy the produced binary to `c/build/getting_started`

To run the C sample (from the root scenario folder `scenarios/getting_started`):

```bash
c/build/getting_started
```