# :point_right: JWT Authentication to Event Grid

| [Create the Client Certificate](#lock-create-the-client-certificate) | [Configure Event Grid Namespaces](#triangular_ruler-configure-event-grid-namespaces) | [Configure Mosquitto](#fly-configure-mosquitto) | [Run the Sample](#game_die-run-the-sample) |

This scenario showcases how to authenticate to Azure Event Grid via JWT authentication using MQTT 5. This scenario is identical to `getting_started` in functionality.

The sample provides step by step instructions on how to perform following tasks:

- Create the resources including client, topic spaces, permission bindings
- Use $all client group, which is the default client group with all the clients in a namespace, to authorize publish and subscribe access in permission bindings
- Create a custom role assignment on the Azure Portal to access Event Grid via Json Web Token (JWT) authentication.
- Create a JWT, which is used to authenticate to Event Grid.
- Connect with MQTT 5.0.0
  - Configure connection settings such as KeepAlive and CleanSession
- Publish messages to a topic
- Subscribe to a topic to receive messages

To keep the scenario simple, a single client called "sample_client" publishes and subscribes to MQTT messages on topics shown in the table.  

|Client|Role|Operation|Topic/Topic Filter|
|------|----|---------|------------------|
|sample_client|publisher|publish|sample/topic1|
|sample_client|subscriber|subscribe|sample/+|


##  :lock: Configure the Json Web Token and AAD Role Assignments

1. Modify the following JSON snippet by adding an Azure subscription Id:

```json
{ 
    "properties": { 
        "roleName": "Event Grid Pub-Sub", 
        "description": "communicate with Event Grid.", 
        "assignableScopes": [ 
            "/subscriptions/<YOUR SUBSCRIPTION ID HERE>"    
        ], 
        "permissions": [ 
            { 
                "actions": [], 
                "notActions": [], 
                "dataActions": [ 
                    "Microsoft.EventGrid/*" 
                ], 
                "notDataActions": [] 
            } 
        ] 
    } 
} 
```
2. Copy the modified snippet and save it locally.
3. In the Azure portal, go to your Resource Group that contains Event Grid and open the Access control (IAM) page. 
4. Click Add and then click Add custom role. This opens the custom roles editor. 
5. On the `Basics` tab, select `Start from JSON`, and upload the modified JSON file you saved locally.
6. Select the `Review and Create` tab and then `Create`.
7. **NOTE:** It is possible that your Azure account may not have room for more custom role assignments. In this instance the current workaround is to create a free Azure account and complete this process while logged in from there.

## :triangular_ruler: Configure Event Grid Namespaces

Ensure to create an Event Grid namespace by following the steps in [setup](../setup).  Event Grid namespace requires registering the client, and the topic spaces to authorize the publish/subscribe permissions.

### Create the Client

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
