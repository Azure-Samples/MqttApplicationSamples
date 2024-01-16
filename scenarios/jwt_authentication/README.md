# :point_right: JWT Authentication to Event Grid

 [Configure Event Grid Namespaces](#triangular_ruler-configure-event-grid-namespaces) |  [Run the Sample](#game_die-run-the-sample) |

This scenario showcases how to authenticate to Azure Event Grid via JWT authentication using MQTT 5. This scenario is identical to `getting_started` in functionality. 

JWT authentication is documented in [Microsoft Entra JWT authentication and Azure RBAC authorization to publish or subscribe MQTT messages](https://learn.microsoft.com/en-us/azure/event-grid/mqtt-client-microsoft-entra-token-and-rbac) from Event Grid documentation.

To keep the scenario simple, a single client called "sample_client" publishes and subscribes to MQTT messages on topics shown in the table.  

|Client|Role|Operation|Topic/Topic Filter|
|------|----|---------|------------------|
|sample_client|publisher|publish|sample/topic1|
|sample_client|subscriber|subscribe|sample/+|

## Prerequisites
This sample involves configuring Event Grid per the specifications in [getting_started](../getting_started). If that sample has not already been set up and run, it should be done before moving onto this one.

## Create the .env file with connection details

The required `.env` files can be configured manually, we provide the script below as a reference to create those files, as they are ignored from git.

```bash
# from folder scenarios/getting_started
source ../../az.env
host_name=$(az resource show --ids $res_id --query "properties.topicSpacesConfiguration.hostname" -o tsv)

echo "MQTT_HOST_NAME=$host_name" > .env
echo "MQTT_USERNAME=sample_client" >> .env
echo "MQTT_CLIENT_ID=sample_client" >> .env
```

## 🔒 Create an Identity in Microsoft Entra ID

Event Grid namespaces supports JWT authentication for Managed Identities and Service principals only:

- **Managed Identity**. You can use a [Managed identity](https://learn.microsoft.com/en-us/entra/identity/managed-identities-azure-resources/overview) provided by many Azure services, such as Azure Container Apps, Azure Container Instances, Azure Kubernetes Services or Azure Web Apps, full list is available [here](https://learn.microsoft.com/en-us/entra/identity/managed-identities-azure-resources/managed-identities-status).

- **Service Principal**. You can create your own [Service Principal](https://learn.microsoft.com/en-us/entra/identity-platform/app-objects-and-service-principals?tabs=browser) by creating an Application Registration in Microsoft Entra ID. 

To create the service principal and the secret using the Azure CLI:

```bash
clientId=$(az ad app create --display-name "MyMqttSamplesApp" --query appId -o tsv)
spId=$(az ad sp create --id $clientId --query id -o tsv)
az ad app credential reset --id $clientId --append
```

take note of the appId, password and tenant values returned from the previous command.

> Note. You can use these values as environment variables for dotnet by using the launchSettings.json file 

## Assign RBAC permissions

In Azure EventGrid Namespaces, assign permissions to the Microsoft Entra ID identity using the  built-in roles `Event Grid Topic Spaces Publisher/Subscriber`.

```bash
# from the root folder
source az.env

az role assignment create \
  --assignee $spId \
  --role "EventGrid TopicSpaces Publisher" \
  --scope $res_id

az role assignment create \
  --assignee $spId \
  --role "EventGrid TopicSpaces Subscriber" \
  --scope $res_id
```

By assigning these roles to an Azure subscription, it allows that subscription to communicate with any Topic Space within an instance of Event Grid owned by the specified Service Principal (e.g., if these roles are assigned at a subscription level, any Topic Space of an Event Grid under the given subscription could be subscribed/published to).

An alternative to using Azure CLI is the Azure Portal:
1. Locate the resource group that contains the desired instance of Event Grid.
2. Navigate to `Access control (IAM)` blade.
3. Under `Role Assignments`, click `Add`, and assign `Event Grid Topic Spaces Publisher/Subscriber` roles.
4. Assign the role to the desired Azure account, and click `Review + Assign`.

## 📐 Configure Event Grid Namespaces (Skip if [getting_started](../getting_started) has already been properly configured)

Ensure to create an Event Grid namespace by following the steps in [setup](../setup).  Event Grid namespace requires registering the client, and the topic spaces to authorize the publish/subscribe permissions.

### Create the Client (Skip if [getting_started](../getting_started) has already been properly configured)

We will use the SubjectMatchesAuthenticationName validation scheme for `sample_client`. Instructions for how to do this can be found in [getting_started](../getting_started). If this has already been done once, it does not have to be done again (unless using a different Azure account).

### Create topic spaces and permission bindings

Run the commands to create the "samples" topic space, and the two permission bindings that provide publish and subscribe access to $all client group on the samples topic space. As for above, the instructions to do this are part of [getting_started](../getting_started) and do not have to be repeated if they have already been done in the Azure account being used to run this sample.

## :game_die: Run the Sample

All samples are designed to be executed from the root scenario folder.

### dotnet

To build the dotnet sample run:

```bash
# from folder scenarios/jwt_authenticaton
dotnet build dotnet/jwt_authentication.sln 
```

To run the dotnet sample:

```bash
 dotnet/jwt_authentication/bin/Debug/net7.0/jwt_authentication
```

## Connecting over WebSocket
To connect using WebSockets, modify client's `ConnectAsync()` call as follows:
```csharp
MqttClientConnectResult connAck = await mqttClient!.ConnectAsync(new MqttClientOptionsBuilder()
    .WithClientId("sample_client")
    //.WithTcpServer(hostname, 8883)
    .WithWebSocketServer(b => b.WithUri($"{hostname}:443/mqtt"))
    .WithProtocolVersion(MQTTnet.Formatter.MqttProtocolVersion.V500)
    .WithAuthentication("OAUTH2-JWT", Encoding.UTF8.GetBytes(jwt.Token))
    .WithTlsOptions(new MqttClientTlsOptions() { UseTls = true })
    .Build());
```

Note that it is required to use port 443 for websocket connections. To learn more about this flow visit the [documentation](https://learn.microsoft.com/azure/event-grid/mqtt-support#connection-flow).
