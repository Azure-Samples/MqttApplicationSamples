# Setup Environment

| [Create CA](#create-ca) | [Configure Event Grid](#configure-event-grid-namespace) | [Configure Mosquitto](#configure-mosquitto-with-tls-and-x509-authentication) | [Development tools](#configure-development-tools) |

Once your environment is configured you can configure your connection settings as environment variables that will be loaded by the [Mqtt client extensions](./mqttclients/README.md)

### Create CA

All samples require a CA to generate the client certificates to connect.

- Follow this link to install the `step cli`: [https://smallstep.com/docs/step-cli/installation/](https://smallstep.com/docs/step-cli/installation/)
- To create the root and intermediate CA certificates run:

```bash
step ca init \
    --deployment-type standalone \
    --name MqttAppSamplesCA \
    --dns localhost \
    --address 127.0.0.1:443 \
    --provisioner MqttAppSamplesCAProvisioner
```

Follow the cli instructions, when done make sure you remember the password used to protect the private keys, by default the generated certificates are stored in:

- `~/.step/certs/root_ca.crt`
- `~/.step/certs/intermediate_ca.crt`
- `~/.step/secrets/root_ca_key`
- `~/.step/secrets/intermediate_ca_key`

## Configure Event Grid Namespace


### Configure environment variables

Create or update `az.env` file under MQTTApplicationSamples folder that includes an existing subscription, an existing resource group, and a new name of your choice for the Event Grid Namespace as follows:

```text
sub_id=<subscription-id>
rg=<resource-group-name>
name=<event-grid-namespace>
res_id="/subscriptions/${sub_id}/resourceGroups/${rg}/providers/Microsoft.EventGrid/namespaces/${name}"
```

To run the `az` cli:
- Install [AZ CLI](https://learn.microsoft.com/cli/azure/install-azure-cli)
- Authenticate using  `az login`.
- If the above does not work use `az login --use-device-code`

```bash
source az.env

az account set -s $sub_id
az resource create --id $res_id --is-full-object --properties '{
  "properties": {
    "isZoneRedundant": true,
    "topicsConfiguration": {
      "inputSchema": "CloudEventSchemaV1_0"
    },
    "topicSpacesConfiguration": {
      "state": "Enabled"
    }
  },
  "location": "westus2"
}'
```

Register the certificate to authenticate client certificates (usually the intermediate)

```bash
source az.env

capem=`cat ~/.step/certs/intermediate_ca.crt | tr -d "\n"`

az resource create \
  --id "$res_id/caCertificates/Intermediate01" \
  --properties "{\"encodedCertificate\" : \"$capem\"}"
```
> Each scenario includes the detailed instructions to configure the namespace resources needed for the scenario.

> [!NOTE]
> For portal configuration, use [this link](https://portal.azure.com/?microsoft_azure_marketplace_ItemHideKey=PubSubNamespace&microsoft_azure_eventgrid_assettypeoptions={"PubSubNamespace":{"options":""}}) and follow [these instructions](https://learn.microsoft.com/en-us/azure/event-grid/mqtt-publish-and-subscribe-portal).

## Configure Mosquitto with TLS and X509 Authentication

Install `mosquitto`

```bash
sudo apt-get update && sudo apt-get install mosquitto -y
```

The local instance of mosquitto requires a certificate to expose a TLS endpoint, the chain `chain.pem` used to create this cert needs to be trusted by clients.

Using the test ca, create a certificate for `localhost`, and store the certificate files in the `_mosquitto` folder.

```bash
# from folder _mosquitto
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
step certificate create localhost localhost.crt localhost.key \
      --ca ~/.step/certs/intermediate_ca.crt \
      --ca-key ~/.step/secrets/intermediate_ca_key \
      --no-password \
      --insecure \
      --not-after 2400h
```

These files are used by  the mosquitto configuration file `tls.conf`

```text
per_listener_settings true

listener 1883
allow_anonymous true

listener 8883
allow_anonymous true
require_certificate true
cafile chain.pem
certfile localhost.crt
keyfile localhost.key
tls_version tlsv1.2
```

To start mosquitto with this configuration file run:

```bash
mosquitto -c tls.conf
```

If you get `Error: Address already in use`, you can run

```bash
ps -ef | grep mosquitto
```

to find the running mosquitto instance, and use the process id returned to end it:

```bash
sudo kill <process id>
```

## Configure development tools

This repo leverages GitHub CodeSpaces, with a preconfigured `.devContainer` that includes all the required tools and SDK, and also a local mosquitto, and the `step` cli.

### dotnet C#

The samples use `dotnet7`, it can be installed in Windows, Linux, or Mac from https://dotnet.microsoft.com/en-us/download

Optionally you can use Visual Studio to build and debug the sample projects.

See [dotnet extensions](./mqttclients/dotnet/README.md) for more details.


### C

We are using standard C, and CMake to build. These are the required tools:
- [CMake](https://cmake.org/download/) Version 3.20 or higher to use CMake presets
- [Mosquitto](https://mosquitto.org/download/) Version 2.0.0 or higher
- [Ninja build system](https://github.com/ninja-build/ninja/releases) Version 1.10 or higher
- GNU C++ compiler
- SSL

An example of installing these tools (other than CMake) is shown below:

```bash
sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa
sudo apt-get install g++-multilib ninja-build libmosquitto-dev libssl-dev
```

See [c extensions](./mqttclients/c/README.md) for more details.

### Python

Python samples have been tested with python 3.10.4, to install follow the instructions from https://www.python.org/downloads/ 
