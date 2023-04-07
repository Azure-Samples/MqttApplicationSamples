# Getting Started Samples

This sample shows how to perform basic MQTT operations: Connect, Publish and Subscribe.

The connection requires client certificates issued by a known CA.

## Locate CA cert files

Generate a CA for this samples as described in [setup](../setup), by default the root and intermediate certificates are stored in:

- `~/.certs/root_ca.crt`
- `~/.certs/intermediate_ca.crt`

## Generate Client Certificates

To generate a client certificate, use the `step certificate create` command from this folder, eg. to create a `vehicle01` certificate issued from the intermediate CA:

```bash
cd getting_started
step certificate create vehicle01 vehicle01.pem vehicle01.key --ca ~/.step/certs/intermediate_ca.crt --ca-key ~/.step/secrets/intermediate_ca_key --no-password --insecure --not-after 2400h
```

## Trust the server certificate used by mosquitto

To establish the TLS connection, the CA needs to be trusted, most MQTT clients allow to specify the ca trust chain as part of the connection, to create a chain file with the root and the intermediate use:

```bash
mkdir _certs && _certs
cat ~/.step/certs/root_ca.crt  ~/.step/certs/intermediate_ca.crt > ca_chain.crt
```

