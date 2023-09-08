#  :point_right: Command (Request/Response)

| [Create Client Certificates](#lock-create-client-certificates) | [Configure Event Grid Namespaces](#triangular_ruler-configure-event-grid-namespaces) | [Configure Mosquitto](#fly-configure-mosquitto) | [Run the Sample](#game_die-run-the-sample) |

This scenario simulates the request-response messaging pattern. Request-response uses two topics, one for the request and one for the response.

Consider a use case where a user can unlock their car from a mobile app. The request to unlock is published on `vehicles/<vehicleId>/commands/unlock/request` and the response of unlock operation is published on `vehicles/<vehicleId>/commands/unlock/response`.

> NOTE: This code is a basic example of the request-response messaging pattern. It is not a secure solution for unlocking a vehicle without further security checks.

## Command Server, Command Client

Every command requires a `server` who implements the command and a `client` who invokes the command, in this case the vehicle is the server and the mobile-app is the client.

|Client|Role|Operation|Topic/Topic Filter|
|------|----|---------|------------------|
|vehicle03|server|sub|vehicles/vehicle03/command/unlock/request|
|vehicle03|server|pub|vehicles/vehicle03/command/unlock/response|
|mobile-app|client|pub|vehicles/vehicle03/command/unlock/request|
|mobile-app|client|sub|vehicles/vehicle03/command/unlock/response|

## Command flow with user properties

To implement the command pattern, the mqtt message used for the request includes additional metadata to control the command flow:

- `Correlation Id` The client includes a new _Guid_ in the message property _CorrelationData_.
- `Response Topic` The client specifies what topic it is expecting the response on, using the message property _ResponseTopic_.
- `ContentType` The client sets the message property _ContentType_ to specify the format used in the binary payload. The server will check this value to make sure it's configured with the proper serializer.
- `Status` The server will set the User Property _status_ on the response, with a HTTP Status code, to let the client know if the execution was successful.

## Payload Format

Messages will be encoded using Protobuf with the following payload.

```proto
syntax = "proto3";

import "google/protobuf/timestamp.proto";

message unlockRequest {
    google.protobuf.Timestamp when = 1;
    string requestedFrom = 2;
}

message unlockResponse {
    bool succeed = 1;
    string errorDetail = 2;
}

service Commands {
	rpc unlock(unlockRequest) returns (unlockResponse)
}
```

## :lock: Create Client Certificates

Run the following step commands to create the client certificates for `vehicle03` and `mobile-app` clients.  The client authentication name is provided in the subject name field of the client certificate.

```bash
# from folder scenarios/command
step certificate create \
    vehicle03 vehicle03.pem vehicle03.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

step certificate create \
    mobile-app mobile-app.pem mobile-app.key \
    --ca ~/.step/certs/intermediate_ca.crt \
    --ca-key ~/.step/secrets/intermediate_ca_key \
    --no-password --insecure \
    --not-after 2400h

```

## :triangular_ruler: Configure Event Grid Namespaces

Event Grid Namespaces requires to register the clients, and the topic spaces to set the client permissions.

### Create the clients

The clients will be created with authentication name same as the value provided earlier in certificate subject field while creating the client certificates.  You can register the 2 clients in the portal or by running the script below:

```bash
# from folder scenarios/command
source ../../az.env

az resource create --id "$res_id/clients/vehicle03" --properties '{
	"authenticationName": "vehicle03",
	"state": "Enabled",
	"clientCertificateAuthentication": {
	    "validationScheme": "SubjectMatchesAuthenticationName"
	},
	"attributes": {
	    "type": "vehicle"
    	},
    	"description": "This is a vehicle client"
}'

az resource create --id "$res_id/clients/mobile-app" --properties '{
	"authenticationName": "mobile-app",
        "state": "Enabled",
        "clientCertificateAuthentication": {
            "validationScheme": "SubjectMatchesAuthenticationName"
        },
        "attributes": {
            "type": "mobile"
   	},
    	"description": "This is a mobile app client"
}'

```

### Configure Permissions with Topic Spaces

```bash
# from folder scenarios/command
az resource create --id "$res_id/topicSpaces/vehiclesCommands" --properties '{
    "topicTemplates": ["vehicles/+/commands/#"]
}'

az resource create --id "$res_id/permissionBindings/vehiclesCmdPub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"vehiclesCommands",
    "permission":"Publisher"
}'

az resource create --id "$res_id/permissionBindings/vehiclesCmdSub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"vehiclesCommands",
    "permission":"Subscriber"
}'
```

### Create the .env files with connection details

The required `.env` files can be configured manually, we provide the script below as a reference to create those files, as they are ignored from git.

```bash
# from folder scenarios/command
source ../../az.env
host_name=$(az resource show --ids $res_id --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "MQTT_HOST_NAME=$host_name" > vehicle03.env
echo "MQTT_USERNAME=vehicle03" >> vehicle03.env
echo "MQTT_CERT_FILE=vehicle03.pem" >> vehicle03.env
echo "MQTT_KEY_FILE=vehicle03.key" >> vehicle03.env

echo "MQTT_HOST_NAME=$host_name" > mobile-app.env
echo "MQTT_USERNAME=mobile-app" >> mobile-app.env
echo "MQTT_CERT_FILE=mobile-app.pem" >> mobile-app.env
echo "MQTT_KEY_FILE=mobile-app.key" >> mobile-app.env
```

## :fly: Configure Mosquitto

To establish the TLS connection, the CA needs to be trusted, most MQTT clients allow to specify the ca trust chain as part of the connection, to create a chain file with the root and the intermediate use:

```bash
# from folder _mosquitto
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
cp chain.pem ../scenarios/command
```
The `chain.pem` is used by mosquitto via the `cafile` settings to authenticate X509 client connections.

```bash
# from folder scenarios/command
echo "MQTT_HOST_NAME=localhost" > vehicle03.env
echo "MQTT_CERT_FILE=vehicle03.pem" >> vehicle03.env
echo "MQTT_KEY_FILE=vehicle03.key" >> vehicle03.env
echo "MQTT_CLIENT_ID=vehicle03" >> vehicle03.env
echo "MQTT_CA_FILE=chain.pem" >> vehicle03.env

echo "MQTT_HOST_NAME=localhost" > mobile-app.env
echo "MQTT_CERT_FILE=mobile-app.pem" >> mobile-app.env
echo "MQTT_KEY_FILE=mobile-app.key" >> mobile-app.env
echo "MQTT_CLIENT_ID=mobile-app" >> mobile-app.env
echo "MQTT_CA_FILE=chain.pem" >> mobile-app.env

```

To use mosquitto without certificates: change the port to 1883 and disable TLS.

```bash
# from folder scenarios/command
echo "MQTT_HOST_NAME=localhost" > vehicle03.env
echo "MQTT_TCP_PORT=1883" >> vehicle03.env
echo "MQTT_USE_TLS=false" >> vehicle03.env
echo "MQTT_CLIENT_ID=vehicle03" >> vehicle03.env

echo "MQTT_HOST_NAME=localhost" > mobile-app.env
echo "MQTT_TCP_PORT=1883" >> mobile-app.env
echo "MQTT_USE_TLS=false" >> mobile-app.env
echo "MQTT_CLIENT_ID=mobile-app" >> mobile-app.env
```

## :game_die: Run the Sample

All samples are designed to be executed from the root scenario folder.

### dotnet

To build the dotnet sample run:

```bash
# from folder scenarios/command
dotnet build dotnet/command.sln 
```

To run the dotnet sample execute each line below in a different shell/terminal.

```bash
 dotnet/command_server/bin/Debug/net7.0/command_server --envFile=vehicle03.env
```
```bash
 dotnet/command_client/bin/Debug/net7.0/command_client --envFile=mobile-app.env
```

### C

To generate the c files to handle the protobuf payload, install protobuf-c-compiler and libprotobuf-dev. Note that you only need these to generate the files, running the sample only requires the libprotobuf-c-dev package.
```bash
sudo apt-get install protobuf-c-compiler libprotobuf-dev
```

Then, to generate the files, run:
```bash
# from the root folder
protoc-c --c_out=./scenarios/command/c/protobuf --proto_path=./scenarios/command/c/protobuf unlock_command.proto google/protobuf/timestamp.proto
```

To build the C sample, run from the root folder:

```bash
cmake --preset=command
cmake --build --preset=command
```

This will generate the produced binary in `scenarios/command/c/build`

To run the C sample, execute each line below in a different shell/terminal.

```bash
# from folder scenarios/command
c/build/command_server vehicle03.env
```
```bash
c/build/command_client mobile-app.env
```

For alternate building/running methods and more information, see the [C documentation](../../mqttclients/c/README.md).


### python

Install the paho client for python:
```bash
pip install paho-mqtt
```

Install internal sample package:
```bash
pip install ../../mqttclients/python

To run the python sample, execute each line below in a different shell/terminal:

```bash
python python/command_receiver.py --env-file=vehicle03.env
```
```bash
python python/command_invoker.py --env-file=mobile-app.env
```
