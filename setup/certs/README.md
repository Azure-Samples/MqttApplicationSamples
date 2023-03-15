# Create X509 certificates

All samples in this repo require a custom CA capable of issuing client certificates, we are using the `step` cli to configure the CA an produce required certificates.

Using `step` cli, from: [https://smallstep.com/docs/step-cli](https://smallstep.com/docs/step-cli)

## Create CA

```bash
step ca init
```

Follow the cli instructions, when done take note of the path to the generated certificates

- root_ca.crt
- intermediate_ca.crt