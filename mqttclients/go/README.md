# Using Go Samples

## Loading Environment Variables
Samples import the code from `ConnectionSettings.go`, which includes the struct `MqttConnectionSettings` and the function `LoadConnectionSettings(path string)` that takes a parameter of type string that specifies the location of a `.env` file. Running samples requires passing this path as a command line argument:

```bash
go run ./program.go <path-to-env-file>
```

## Relevant Libraries
- MQTT V5 [Paho Client for Go](https://github.com/eclipse/paho.golang)
- [Godotenv](https://github.com/joho/godotenv) to load environment variables
