# Telemetry (Fan-in)

## Create Client Certificates

vehicle01

```bash

step certificate create vehicle01 vehicle01.pem vehicle01.key --ca ~/.step/certs/intermediate_ca.crt --ca-key ~/.step/secrets/intermediate_ca_key --no-password --insecure --not-after 2400h
```