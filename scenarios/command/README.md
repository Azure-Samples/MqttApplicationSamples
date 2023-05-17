#  :point_right: Command (Request/Response)

| [Create the Client Certificates](#create-client-certificates) | [Configure Event Grid Namespaces](#configure-event-grid-namespaces) | [Configure Mosquitto](#configure-mosquitto) | [Run the Sample](#run-the-sample) |

This scenario simulates the request-response messaging pattern. Request-response uses two topics, one for the request and one for the response.

Consider a use case where a user can unlock their car from a mobile app. The request to unlock is published on `vehicles/<vehicleId>/commands/unlock` and the response of unlock operation is published on `vehicles/<vehicleId>/commands/unlock/response`.

|Client|Role|Operation|Topic/Topic Filter|
|------|----|---------|------------------|
|vehicle03|producer|sub|vehicles/vehicle1/commands/unlock|
|vehicle03|producer|pub|vehicles/vehicle1/commands/unlock/response|
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

## :lock: Create Client Certificates

Run the following step commands to create the client certificates for `vehicle03` and `mobile-app` clients.  The client authentication name is provided in the subject name field of the client certificate.

```bash
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
source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"

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
az resource create --id "$res_id/topicSpaces/vehiclesCommands" --properties '{
    "topicTemplates": ["vehicles/+/command/#"]
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
source ../../az.env
res_id="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"
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
cat ~/.step/certs/root_ca.crt ~/.step/certs/intermediate_ca.crt > chain.pem
```
The `chain.pem` is used by mosquitto via the `cafile` settings to authenticate X509 client connections.

```bash
echo "MQTT_HOST_NAME=localhost" > vehicle03.env
echo "MQTT_CERT_FILE=vehicle03.pem" >> vehicle03.env
echo "MQTT_KEY_FILE=vehicle03.key" >> vehicle03.env
echo "MQTT_CA_FILE=chain.pem" >> vehicle03.env

echo "MQTT_HOST_NAME=localhost" > mobile-app.env
echo "MQTT_CERT_FILE=mobile-app.pem" >> mobile-app.env
echo "MQTT_KEY_FILE=mobile-app.key" >> mobile-app.env
echo "MQTT_CA_FILE=chain.pem" >> mobile-app.env

```

To use mosquitto without certificates: change the port to 1883, disable TLS and set the CA_FILE

```bash
echo "MQTT_HOST_NAME=localhost" > vehicle03.env
echo "MQTT_TCP_PORT=1883" >> vehicle03.env
echo "MQTT_USE_TLS=false" >> vehicle03.env
echo "MQTT_CLIENT_ID=vehicle03" >> vehicle03.env
```

## :game_die: Run the Sample

All samples are designed to be executed from the root scenario folder.

### dotnet

To build the dotnet sample run:

```bash
dotnet build dotnet/command.sln 
```

To run the dotnet sample execute each line below in a different shell/terminal.

```bash
 dotnet/command_producer/bin/Debug/net7.0/command_producer --envFile=vehicle03.env
 dotnet/command_consumer/bin/Debug/net7.0/command_consumer --envFile=mobile-app.env
```
