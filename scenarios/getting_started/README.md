# :point_right: Getting Started

| [Create the Client Certificate](#lock-create-the-client-certificate) | [Configure Event Grid Namespaces](#triangular_ruler-configure-event-grid-namespaces) | [Configure Mosquitto](#fly-configure-mosquitto) | [Run the Sample](#game_die-run-the-sample) |

This scenario showcases how to create resources such as client, topic spaces, and permission bindings to publish and subscribe MQTT messages.

The sample provides step by step instructions on how to perform following tasks:

- Create client certificate, which is used to authenticate the client connection
- Create the resources including client, topic spaces, permission bindings
- Use $all client group, which is the default client group with all the clients in a namespace, to authorize publish and subscribe access in permission bindings
- Connect with MQTT 3.1.1
  - Validate TLS certificate enforcing TLS 1.2
  - Authenticate the client connection using client certificates
  - Configure connection settings such as KeepAlive and CleanSession
- Publish messages to a topic
- Subscribe to a topic to receive messages

To keep the scenario simple, a single client called "sample_client" publishes and subscribes to MQTT messages on topics shown in the table.  

|Client|Role|Operation|Topic/Topic Filter|
|------|----|---------|------------------|
|sample_client|publisher|publish|sample/topic1|
|sample_client|subscriber|subscribe|sample/+|


##  :lock: Create the client certificate

Using the CA files, as described in [setup](../../Setup.md), create a certificate for `sample_client` client.  Client certificate is created with subject name as "sample_client".  This must match the authentication name of the client.

```bash
# from folder scenarios/getting_started
step certificate create \
    sample_client sample_client.pem sample_client.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h
```

## :triangular_ruler: Configure Event Grid Namespaces

Ensure to create an Event Grid namespace by following the steps in [setup](../setup).  Event Grid namespace requires registering the client, and the topic spaces to authorize the publish/subscribe permissions.

### Create the Client

We will use the SubjectMatchesAuthenticationName validation scheme for `sample_client` to create the client from the portal or with the script:

```bash
# from folder scenarios/getting_started
source ../../az.env

az resource create --id "$res_id/clients/sample_client" --properties '{
    "authenticationName": "sample_client",
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "validationScheme": "SubjectMatchesAuthenticationName"
    },
    "attributes": {
        "type": "sample-client"
    },
    "description": "This is a test publisher client"
}'
```

### Create topic spaces and permission bindings
Run the commands to create the "samples" topic space, and the two permission bindings that provide publish and subscribe access to $all client group on the samples topic space.

```bash
# from folder scenarios/getting_started
source ../../az.env

az resource create --id "$res_id/topicSpaces/samples" --properties '{
    "topicTemplates": ["sample/#"]
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
# from folder scenarios/getting_started
source ../../az.env
host_name=$(az resource show --ids $res_id --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "MQTT_HOST_NAME=$host_name" > .env
echo "MQTT_USERNAME=sample_client" >> .env
echo "MQTT_CLIENT_ID=sample_client" >> .env
echo "MQTT_CERT_FILE=sample_client.pem" >> .env
echo "MQTT_KEY_FILE=sample_client.key" >> .env
```

## Configure IoTMQ



## :fly: Configure Mosquitto

To establish the TLS connection, the CA needs to be trusted, most MQTT clients allow to specify the ca trust chain as part of the connection, to create a chain file with the root and the intermediate use:

```bash
# from folder _mosquitto
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
cp chain.pem ../scenarios/getting_started
```
The `chain.pem` is used by mosquitto via the `cafile` settings to authenticate X509 client connections.

```bash
# from folder scenarios/getting_started
echo "MQTT_HOST_NAME=localhost" > .env
echo "MQTT_CLIENT_ID=sample_client" >> .env
echo "MQTT_CERT_FILE=sample_client.pem" >> .env
echo "MQTT_KEY_FILE=sample_client.key" >> .env
echo "MQTT_CA_FILE=chain.pem" >> .env
```

To use mosquitto without certificates

```bash
# from folder scenarios/getting_started
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
# from folder scenarios/getting_started
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
# from the root of the repo
cmake --preset=getting_started
cmake --build --preset=getting_started
```
This will generate the produced binary in `scenarios/getting_started/c/build/`

To run the C sample:

```bash
# from folder scenarios/getting_started
c/build/getting_started
```

For alternate building/running methods and more information, see the [C documentation](../../mqttclients/c/README.md).

### Python

#### Create a virtual environment

It is recommended to use a Python virtual environment for this sample. To avoid creating a python virtual environment, skip to [run the sample](#run-the-sample).

Create and activate the virtual environment:
```bash
python -m venv <desired location for virtual environment>
source  <virtual environment directory>/bin/activate
```
By activating the virtual environment, all the python dependencies will be installed only within the virtual environment rather than system-wide.

Once you are done using the virtual environment, you can deactivate by running:
```bash
deactivate
```

#### Run the sample
*The commands below assume you are in the MqttApplicationSamples/scenarios/getting_started directory.*

Install the paho client for python:
```bash
pip install paho-mqtt
```

Install internal sample package:
```bash
pip install ../../mqttclients/python
```

Run the sample using settings from an envfile. This assumes the env file is placed inside the `getting_started` directory and the sample is also ran from there.
```bash
# from folder scenarios/getting_started
python python/getting_started.py --env-file <path to .env file>
```

### Go

To build the Go sample run:

```bash
# from folder scenarios/getting_started
$GOROOT/bin/go build -C go/ -o bin/
```

Run the sample using settings from an envfile

```bash
# from folder scenarios/getting_started
 go/bin/getting_started .env
```

### TypeScript
To build the TypeScript sample run:
>Note: The scenario should already be built from the initial `npm i` command at the root.

```bash
# from folder scenarios/getting_started
npm run build --prefix ./ts/gettingStarted
```

The sample can be run and debugged either in the [Visual Studio Code IDE](https://code.visualstudio.com/), or from the command line. This will use the `.env` file created in the steps above.

To run the sample from Visual Studio Code, select the Run and Debug option from the left pane then select the "TypeScript Getting Started" configuration from the Run and Debug dropdown menu. Then just use F5 or click on the green play button.

To run the sample from the command line:
```bash
# from folder scenarios/getting_started
node ./ts/gettingStarted/dist/index.js --env-file .env
```

To see detailed MQTT.js debug logging configure the DEBUG environment variable before running the sample.

Using Visual Studio Code, simply add another entry to the .env file:
```bash
DEBUG=mqttjs*
```

Using the command line:
```bash
export DEBUG=mqttjs* && node ./ts/gettingStarted/dist/index.js --env-file .env
```

### Rust
*The commands below assume you are in the MqttApplicationSamples/scenarios/getting_started directory.*
*Assumes that we have cargo and rust installed as well.*

To compile the sample run:
```bash
cargo build --manifest-path rust/getting_started/Cargo.toml
```
TO run the sample do:
```bash
cargo run --manifest-path rust/getting_started/Cargo.toml
```

To set debug level on mqtt client do
```bash
RUST_LOG=debug cargo run --manifest-path rust/getting_started/Cargo.toml
```
