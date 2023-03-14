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
  - Configure connection settings such as KeepAlive and CleanSession or ConnectionTimeout
- Publish 
  - Send messages encoded with different payload encodings: JSON/UTF8 and Protobuf using QoS0 and QoS1
- Subscribe
  - Subscribe to a topic to receive and decode messages
 
## MQTT 5 Samples

Show how use some of the new MQTT5 features available in E4K and AzPubSub

- Error Handling inspecting ReasonCodes
- User Properties in messages 
- Request Response with correlation
- Message expiry

# Scenario Samples

The following samples are implementations of the core PubSub patterns used with MQTT Brokers. Each scenario might require a different number of producers and consumers, these are loosely coupled actors that interact with the broker using some topic structure and known message payloads.

Each scenario requires to configure the brokers:

- Configure Authentication: mTLS certificates and clients
- Configure Authorization: Define which client(s) can interact with which topic(s)

## Telemetry (Fan-In)

This scenario shows how multiple clients send data (the producers) to a different set of topics that can be consumed by a single application (the consumer).

Consider a use case where a backend solution needs to identify the location of vehicles on a map. Vehicles should be prohibited from listening to other vehicles location on their behalf.

|Client|Role|Operation|Topic/Topic Filter|
|------|----|---------|------------------|
|vehicle1|producer|pub|vehicles/vehicle1/position|
|vehicle2|producer|pub|vehicles/vehicle2/position|
|map-app|consumer|sub|vehicles/+/position|

Messages will use [GeoJSON](https://geojson.org) to represent the coordinates.

```json
{
    "type": "Point",
    "coordinates": [125.6, 10.1]
}
```

##  Command (Request/Response)

This scenario simulates the request-response messaging pattern. Request-response uses two topics, one for the request and one for the response.

Consider a use case where a user can unlock their car from a mobile app. The request to unlock are published on `vehicles/<vehicleId>/commands/unlock` and the response of unlock operation are published on `vehicles/<vehicleId>/commands/unlock/response`.

|Client|Role|Operation|Topic/Topic Filter|
|------|----|---------|------------------|
|vehicle1|producer|sub|vehicles/vehicle1/commands/unlock|
|vehicle1|producer|pub|vehicles/vehicle1/commands/unlock/response|
|mobile-app|consumer|pub|vehicles/vehicle1/commands/unlock|
|mobile-app|consumer|sub|vehicles/vehicle1/commands/unlock/response|

Messages will be encoded using Protobuf with the following payload.

```proto
syntax = "proto3";

import "google/protobuf/timestamp.proto";

message unlockRequest {
    google.protobuf.Timestamp when = 1;
    string requestedFrom = 2;
}

message unlockResponse {
    bool succeed =1 ;
    string errorDetail = 2;
}

service Commands {
	rpc unlock(unlockRequest) returns (unlockResponse)
}
```

## Command (Fan-Out)

This scenario shows how multiple clients can receive notifications from the same topic, this pattern can be leveraged for use cases such as sending alerts. Consider the use case where a fleet management service needs to send a weather alerts to all the vehicles in the fleet.

|Client|Role|Operation|Topic/Topic Filter|
|------|----|---------|------------------|
|vehicle1|consumer|sub|fleets/alerts/#|
|vehicle2|consumer|sub|fleets/alerts/#|
|fleet-mgr|producer|pub|fleets/alerts/weather1|

Messages will be encoded with custom JSON schema

```json
{
    "weatherAlert" : {
        "starts" : "2023-03-14T22:00:000Z",
        "ends" : "2023-03-15T22:00:000Z",
        "description" : "High Winds expected"
    }
}
```
