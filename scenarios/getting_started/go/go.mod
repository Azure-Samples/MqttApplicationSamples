module mqttapplicationsamples/getting_started

go 1.21.3

require github.com/eclipse/paho.golang v0.12.0

require mqttapplicationsamples/ConnectionSettings v1.0.0

require (
	github.com/joho/godotenv v1.5.1 // indirect
	golang.org/x/sync v0.4.0 // indirect
)

replace mqttapplicationsamples/ConnectionSettings => ../../../mqttclients/go/
