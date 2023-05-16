# MQTT Application Samples

Guidance to build Pub/Sub applications targeting MQTT Brokers in different programming languages.

| [Setup](./Setup.md) | [Getting Started](./scenarios/getting_started/) | [Telemetry](./scenarios/telemetry/) | [Command](./scenarios/command/) |

## Prerequisites

To run this samples you need an MQTT Broker configured with mTLS to authenticate clients with X509 certificates, we provide instructions for:

- Azure Event Grid Namespaces 
- Mosquitto for local development

Samples are provided in different programming languages: C#, Python and C.

> Note: The samples are optimized to run in Linux, or WSL in Windows. (To run in native Windows you must adapt the scripts to use Windows paths)

- The samples are located in the folder [scenarios](./scenarios/) with sub folders for each language.
- To configure the MQTT connection the samples use `.env` files, with variables to specify the hostname, port, certificates, etc.. 
- The `.env` files must be located in the scenario folder, eg `scenarios/getting_started` and can be reused across samples/languages, including the client certificates.
- Each sample must be executed from the scenario folder
- The environment variables are described [here](./mqttclients/README.md)

See [Setup](./Setup.md) for detailed instructions.

## Language specific instructions

Each language requires developer tools, such as compilers and SDKs to build and run the samples:

- [dotnet](./mqttclients/dotnet/README.md)
- [C](./mqttclients/c/README.md)
- Python (TBD)

# Scenarios

These samples implement PubSub patterns used in MQTT Applications.

Each scenario involves a different number of producers and consumers. These producers and consumers are loosely coupled actors that interact with the MQTT broker using a specific topic structure and known message payloads.

Each scenario requires the following configurations:

- Configure Authentication: mTLS certificates and clients.
- Configure Authorization: Define which client(s) can interact with which topic(s).

Follow the instructions in each scenario README to configure the clients.

| Scenario | Description | dotnet | C | python |
| -------- | ------------|--------|---|------- |
| [Getting Started](./scenarios/getting_started/) | This quick start scenario simulates basic MQTT tasks.| :white_check_mark:|:white_check_mark:|:white_check_mark:|
| [Telemetry](./scenarios/telemetry/)  | Multiple clients (producers) send data that is received from a single client (consumer) | :white_check_mark:|:white_check_mark:|:asterisk:|
| [Command](./scenarios/command/)  | Implements the request-response pattern using MQTT5 features  | :white_check_mark:|:asterisk:|:asterisk:|
| [Alert](./scenarios/alert/)  | Multiple clients are subscribed to a single topic, that can be use to _fan_out_ an alert with a single message.  | :asterisk:|:asterisk:|:asterisk:|

>note: :asterisk: in progress