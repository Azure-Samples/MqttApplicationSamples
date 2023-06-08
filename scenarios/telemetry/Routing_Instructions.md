# Routing Telemetry
Event Grid allows you to route your MQTT messages to Azure services or webhooks for further processing. Accordingly, you can build end-to-end solutions by leveraging your IoT data for data analysis, storage, and visualizations, among other use cases.

The routing configuration enables you to send all your messages from your clients to an [Event Grid custom topic](https://learn.microsoft.com/en-us/azure/event-grid/custom-topics), and configuring [Event Grid event subscriptions](https://learn.microsoft.com/en-us/azure/event-grid/subscribe-through-portal) to route the messages from that custom topic to the [supported event handlers](https://review.learn.microsoft.com/en-us/azure/event-grid/event-handlers). 

These instructions guide you to route your filtered MQTT messages from your Event Grid namespace to an [Azure Event Hubs](https://learn.microsoft.com/en-us/azure/event-hubs/event-hubs-about) for further processing. Consider a use case where you want to route the telemetry from only vehicle1.

## Configure resources
Configure these resources after the Event Grid namespace configuration to enable the routing flow.

### Create an Event Hubs instance
Use [these instructions](https://learn.microsoft.com/en-us/azure/event-hubs/event-hubs-quickstart-cli) to create an Event Hubs instance.

### Configure the Event Grid topic
- Configure your Event Grid Topic where your messages will be routed. The region of the topic needs to match the region of your namespace.
```bash
az eventgrid topic create 
--name {EG custom topic name} \
-l {region name} \
-g $rg \
--input-schema cloudeventschemav1_0
```
- Assign EventGrid Data Sender role on the Event Grid topic to your principal ID 
```bash
az role assignment create 
--assignee "{Your Principal ID}" \
--role "EventGrid Data Sender" \
--scope "/subscriptions/$sub_id/resourcegroups/$rg/providers/Microsoft.EventGrid/topics/{EG Custom Topic Name}"
```
> Note: 
> You can find your principal ID using the command: az ad signed-in-user show

### Configure your Event Grid event subcription

Create an Event Grid event subscription to route these messages from the Event Grid topic to your Event Hubs instance. The subscription uses the "Subject Begins With" filter that filters on the MQTT topic used in the MQTT messages.

```bash
az eventgrid event-subscription create --name contosoEventSubscription \
--source-resource-id "/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/topics/{Your Event Grid Topic Name}" \
--endpoint-type eventhub \
--endpoint /subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventHub/namespaces/{Event Hub Namespace Name}/eventhubs/{Event Hub Name}
--event-delivery-schema cloudeventschemav1_0 \
--subject-begins-with vehicles/vehicle1
```

### Configure routing in the Event Grid Namespace
Set the routing configuration on the Event Grid referring to the Event Grid topic.

```bash
az resource update --id $res_id --is-full-object --properties '{
  "properties": {
    "isZoneRedundant": true,
    "topicsConfiguration": {
      "inputSchema": "CloudEventSchemaV1_0"
    },
    "topicSpacesConfiguration": {
      "state": "Enabled"
      "routeTopicResourceId": "/subscriptions/{Subscription ID}/resourceGroups/{Resource Group ID}/providers/Microsoft.EventGrid/topics/{EG Custom Topic Name}"
    }
  },
  "location": "eastus2euap"
}'
```
### View the routed MQTT messages in Azure Event Hubs
After you run the samples to send the MQTT messages, follow these steps to view the routed MQTT messages in Azure Event Hubs using Azure Stream Analytics query
- Navigate to the Event Hubs instance on the Azure portal.
- Go to [**Process data**](https://learn.microsoft.com/en-us/azure/event-hubs/process-data-azure-stream-analytics) using Azure Stream Analytics. 
- Observe the routed MQTT messages based on the default query.
