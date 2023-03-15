# Configure Mosquitto for local development

## Install Mosquitto

https://mosquitto.org/download/

## Sample conf file with X509 support

Using the test ca, create certificates for `localhost`. 

```bash
step certificate create localhost localhost.crt localhost.key --ca ~/.step/certs/intermediate_ca.crt --ca-key ~/.step/secrets/intermediate_ca_key
```

Copy the certificate files: root_ca.crt, localhost.crt and localhost.key to a local folder where mosquitto should start.

> Mosquitto requires to rename the `cafile` to use the .PEM extension

`local.conf`

```text
per_listener_settings true

listener 1883
allow_anonymous true

listener 8883
allow_anonymous true
require_certificate true
cafile root_ca.pem
certfile localhost.crt
keyfile localhost.key
tls_version tlsv1.2
```

Run mosquitto with:

```bash
mosquitto -c local.conf
```
