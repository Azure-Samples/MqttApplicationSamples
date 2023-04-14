# Setup Environment

These samples can work with any MQTT Broker configured to accept authenticated connections with X509 certificates. This document describes how to configure:

- Azure Event Grid Namespaces
- Mosquitto

> To create the server and client certificates use the `step cli` [https://smallstep.com/docs/step-cli/installation/](https://smallstep.com/docs/step-cli/installation/)

## Create CA

All samples require to register the root CA certificates used to generate the client certs.

```bash
step ca init --deployment-type standalone --name MqttAppSamplesCA --dns localhost --address 127.0.0.1:443 --provisioner MqttAppSamplesCAProvisioner
```

Follow the cli instructions, when done take note of the path to the generated certificates and keys, by default those are stored in:

- `~/.step/certs/root_ca.crt`
- `~/.step/certs/intermediate_ca.crt`
- `~/.step/secrets/root_ca_key`
- `~/.step/secrets/intermediate_ca_key`

## Configure Event Grid Namespace

Access the Azure portal by using [this link](https://portal.azure.com/?microsoft_azure_marketplace_ItemHideKey=PubSubNamespace&microsoft_azure_eventgrid_assettypeoptions={"PubSubNamespace":{"options":""}}).

1. Create new resource, search for Event Grid
2. Select `Event Grid Namespace`
3. Select your resource group and deploy to a supported region (US Central EUAP)
4. Navigate to the new created resource
5. Configure the CA certificate by registering the intermediate ca cert file (~/.step/certs/intermediate_ca.crt)
6. Configure Clients, TopicSpaces and Permissions

> Each scenario includes detailed instructions to configure TopicSpaces, Clients and Permissions, along with `az cli` scripts.

## Configure Mosquitto with TLS and X509 Authentication

Using the test ca, create certificates for `localhost`. 

```bash
cd _mosquitto
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
step certificate create localhost localhost.crt localhost.key --ca ~/.step/certs/intermediate_ca.crt --ca-key ~/.step/secrets/intermediate_ca_key --no-password --insecure --not-after 2400h
```

These files are used by mosquitto:

```
mosquitto -c tls.conf
```

`tls.conf`

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
