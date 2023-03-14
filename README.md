# MQTT Application Samples

Guidance to build Pub/Sub applications targeting Edge and Cloud MQTT Brokers in different programming languages.

## Prerequisites

To run this samples you need an MQTT Broker configured with mTLS to authenticate clients with X509 certificates, we provide instructions for:

- Azure DMQTT (aka E4K), for Kubernetes
- Azure Event Grid PubSub (aka AzPubSub)
- Mosquitto for reference

Samples are provided in different programming languages: C#, Python and C.

### Broker and Certificates setup

- MQTT Broker
  - E4K can be installed in a K8s cluster such as AKS EE, or AKS
  - AzPubSub can be deployed in Azure following this instructions
  - Mosquitto can run in Windows, WSL, Docker, or K8s

- A X509 certificate chain with one Root certificate - to be used as a CA- and different Leaf certificates. Intermediate CAs are optional. Certificate can use ECC and/or RSA keys
  - Create CA Certificate
  - Create TLS Certificate
  - Create mTLS (aka client) Certificates

- Configure Broker Authentication to use X509
  - Configure E4k with X509 
  - Configure AzPub with X509
  - Configure Mosquitto with X509

- Programming language environment
  - dotnet 6
  - Python 3
  - C (CMake 3.14 + Ninja in WSL, Linux)


## Getting Started Samples

Getting started samples show how to perform basic MQTT tasks:

- Connect with MQTT 3.1.1
  - Validate TLS certificate enforcing TLS 1.2
  - Authenticate with client certificates
  - Configure connection settings such as KeepAlive and CleanSession
- Publish 
  - Send messages encoded with different payload encodings: JSON/UTF8 and Protobuf using QoS0 and QoS1
- Subscribe
  - Subscribe to a topic to receive and decode messages
 
## MQTT 5 Samples

Create an advanced sample using MQTT5 to make use of v5 features

- Error Handling inspecting ReasonCodes
- User Properties in messages 
- Request Response with correlation

# Scenario Samples




