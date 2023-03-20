# Setup Environemnt

These samples are intended to run in 3 MQTT environments:

- Cloud with Event Grid
- Edge with E4K
- Local with mosquitto


## Create CA

All samples require to register the root CA certificates used to generate the client certs.

```bash
step ca init
```

Follow the cli instructions, when done take note of the path to the generated certificates and keys, by default those are stored in:

- `~/.step/certs/root_ca.crt`
- `~/.step/certs/intermediate_ca.crt`
- `~/.step/secrets/root_ca_key`
- `~/.step/secrets/intermediate_ca_key`

## Configure Event Grid Namepsaces

Access the Azure portal by using [this link](https://portal.azure.com/?microsoft_azure_marketplace_ItemHideKey=PubSubNamespace&microsoft_azure_eventgrid_assettypeoptions={"PubSubNamespace":{"options":""}}).

1. Create new resource, search for Event Grid
2. Select `Event Grid Namespace`
3. Select your resource group and deploy to a supported region (US Central EUAP)
4. Navigate to the new created resource
5. Configure the CA certificate
6. Configure Clients, TopicSpaces and Permissions

## Configure E4K with TLS and X509 certificates

Deploy E4K with this configuration in values.yaml

```yaml
e4kdmqtt:
  broker:
    frontend:
      ports:
        - port: 1883
        - port: 8883
          tls:
            secret: "test-server-cert"
  authenticationMethods:
    - method: x509
```

To validate client certificates, E4K must trust the CA. Create the chain with:

```bash
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
```

Deploy E4K with

```bash
helm install e4k oci://e4kpreview.azurecr.io/helm/az-e4k --version 0.2.0 -f ./values.yaml --set-file e4kdmqtt.authentication.x509.clientTrustedRoots=chain.pem
```

Create a server certificate for your cluster (need to replace the Service IP, than can be obtained with `kubectl get svc`)

```bash
step certificate create azedge-dmqtt-frontend azedge-dmqtt-frontend.crt azedge-dmqtt-frontend.key --profile leaf --ca ~/.step/certs/intermediate_ca.crt --ca-key ~/.step/secrets/intermediate_ca_key --not-after 2400h --no-password --insecure --san azedge-dmqtt-frontend --san <DMQTT-Service-IP>
```

To register the certificate in the K8s cluster create the secret with the service certificate and key

```bash
kubectl create secret tls test-server-cert --cert azedge-dmqtt-frontend.crt --key azedge-dmqtt-frontend.key
```


## Configure Mosquitto with TLS and X509 Authentication

Using the test ca, create certificates for `localhost`. 

```bash
step certificate create localhost localhost.crt localhost.key --ca ~/.step/certs/intermediate_ca.crt --ca-key ~/.step/secrets/intermediate_ca_key --no-password --insecure --not-after 2400h
```

Copy the certificate files: root_ca.crt, localhost.crt and localhost.key to a local folder where mosquitto should start.

> Mosquitto requires to rename the `cafile` to use the .PEM extension

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

Run mosquitto with:

```bash
mosquitto -c tls.conf
```
