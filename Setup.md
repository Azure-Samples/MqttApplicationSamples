# Setup Environment

| [Create CA](#create-ca) | [Configure Event Grid](#configure-event-grid-namespace) | [Configure Mosquitto](#configure-mosquitto-with-tls-and-x509-authentication) | [Development tools](#configure-development-tools) |

These samples can work with any MQTT Broker configured to accept authenticated connections with X509 certificates. This document describes how to configure:

- Azure Event Grid Namespaces
- Mosquitto

Once your environment is configured you can configure your connection settings as environment variables that will be loaded by the [Mqtt client extensions](./mqttclients/README.md)

### Create CA

All samples require a CA to generate the client certificates to connect.

> To create the certificates use `step cli` [https://smallstep.com/docs/step-cli/installation/](https://smallstep.com/docs/step-cli/installation/)

To create the root and intermediate CA certificates run:

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

Access the Azure portal by using [this link](https://portal.azure.com/?microsoft_azure_marketplace_ItemHideKey=PubSubNamespace&microsoft_azure_eventgrid_assettypeoptions={"PubSubNamespace":{"options":""}}).

1. Create new resource, search for Event Grid
2. Select `Event Grid Namespace`
3. Select your resource group and deploy to a [supported region](https://github.com/Azure/MQTTBrokerPrivatePreview#concepts)
4. Navigate to the new created resource
5. Select configuration and enable MQTT
6. Configure the CA certificate by registering the intermediate ca cert file (~/.step/certs/intermediate_ca.crt)
7. Configure Clients, TopicSpaces and Permissions

> Each scenario includes detailed instructions to configure TopicSpaces, Clients and Permissions, along with `az cli` scripts.

Create or update `az.env` at the repo root with subscription, resource group, and the name for the EventGrid Namespace.

```text
sub_id=<subscription-id>
rg=<resource-group-name>
name=<event-grid-namespace>
res_id="/subscriptions/${sub_id}/resourceGroups/${rg}/providers/Microsoft.EventGrid/namespaces/${name}"
```

## Create the Event Grid Namespace instance

Install [Azure CLI](https://learn.microsoft.com/cli/azure/install-azure-cli)

To run the `az` cli, make sure you are authenticated `az login` with an account that has permissions on the selected subscription.

```bash
source az.env

az account set -s $sub_id
az resource create --id $res_id --is-full-object --properties '{
  "properties": {
    "topicsConfiguration": {
      "inputSchema": "CloudEventSchemaV1_0"
    },
    "topicSpacesConfiguration": {
      "state": "Enabled"
    }
  },
  "location": "eastus2euap"
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


## Configure Mosquitto with TLS and X509 Authentication

The local instance of mosquitto requires a certificate to expose a TLS endpoint, the chain `chain.pem` used to create this cert needs to be trusted by clients.

Using the test ca, create a certificate for `localhost`, and store the certificate files in the `_mosquitto` folder.

```bash
cd _mosquitto
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

## Configure development tools

This repo leverages GitHub CodeSpaces, with a preconfigured `.devContainer` that includes all the required tools and SDK, and also a local mosquitto, and the `step` cli.

### dotnet C#

The samples use `dotnet7`, it can be installed in Windows, Linux or Mac from https://dotnet.microsoft.com/en-us/download

Optionally you can use Visual Studio to build and debug the sample projects.

See [dotnet extensions](./mqttclients/dotnet/README.md) for more details.


### C

We are using standard C, and [CMake](https://cmake.org/download/) to build. You can install the additionally required tools with:

```bash
sudo apt-get install g++-multilib ninja-build libmosquitto-dev libssl-dev
```

See [c extensions](./mqttclients/c/README.md) for more details.

### Python

Python samples have been tested with python 3.10.4, to install follow the instructions from https://www.python.org/downloads/ 


