sub_id=""
rg=""
name=""

resid="/subscriptions/$sub_id/resourceGroups/$rg/providers/Microsoft.EventGrid/namespaces/$name"

az account set -s $sub_id
az resource create --id $resid --is-full-object --properties '{
  "properties": {
    "topicsConfiguration": {
      "inputSchema": "CloudEventSchemaV1_0"
    },
    "topicSpacesConfiguration": {
      "state": "Enabled"
    }
  },
  "location": "centraluseuap",
  "tags": {
    "demo": "rido-bb"
  }
}'

az resource create --id "$resid/caCertificates/caCert" --properties '{
  "encodedCertificate": ""
}'

az resource create --id "$resid/clients/client1" --properties '{
    "state": "Enabled",
    "authentication": {
        "certificateSubject": {
            "commonName": "client1"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'

az resource create --id "$resid/topicSpaces/samples" --properties '{
    "topicTemplates": ["samples/#"],
    "subscriptionSupport": "LowFanout"
}'

az resource create --id "$resid/permissionBindings/samplesPub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"samples",
    "permission":"Publisher"
}'

az resource create --id "$resid/permissionBindings/samplesSub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"samples",
    "permission":"Subscriber"
}'