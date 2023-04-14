source .env

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
  "location": "eastus2euap"
}'

capem=`cat ~/.step/certs/intermediate_ca.crt | tr -d "\n"`
az resource create --id "$resid/caCertificates/MqttAppSamplesCA" --properties "{\"encodedCertificate\" : \"$capem\"}"

az resource create --id "$resid/clients/vehicle01" --properties '{
    "state": "Enabled",
    "clientCertificateAuthentication": {
        "certificateSubject": {
            "commonName": "vehicle01"
        }
    },
    "attributes": {},
    "description": "This is a test publisher client"
}'

az resource create --id "$resid/topicSpaces/sample" --properties '{
    "topicTemplates": ["sample/#"],
    "subscriptionSupport": "LowFanout"
}'

az resource create --id "$resid/permissionBindings/samplesPub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"sample",
    "permission":"Publisher"
}'

az resource create --id "$resid/permissionBindings/samplesSub" --properties '{
    "clientGroupName":"$all",
    "topicSpaceName":"sample",
    "permission":"Subscriber"
}'