package ConnectionSettings

import (
	"os"
	"strconv"

	"github.com/joho/godotenv"
)

type MqttConnectionSettings struct {
	Hostname        string
	TcpPort         int
	UseTls          bool
	CleanSession    bool
	CaFile          string
	CertFile        string
	KeyFile         string
	KeyFilePassword string
	KeepAlive       uint16
	ClientId        string
	Username        string
	Password        string
}

var mqttSettingNames = [12]string{
	"MQTT_HOST_NAME",
	"MQTT_TCP_PORT",
	"MQTT_USE_TLS",
	"MQTT_CLEAN_SESSION",
	"MQTT_KEEP_ALIVE_IN_SECONDS",
	"MQTT_CLIENT_ID",
	"MQTT_USERNAME",
	"MQTT_PASSWORD",
	"MQTT_CA_FILE",
	"MQTT_CERT_FILE",
	"MQTT_KEY_FILE",
	"MQTT_KEY_FILE_PASSWORD",
}

var defaults = map[string]string{
	"MQTT_TCP_PORT":              "8883",
	"MQTT_USE_TLS":               "true",
	"MQTT_CLEAN_SESSION":         "true",
	"MQTT_KEEP_ALIVE_IN_SECONDS": "30",
}

func parseIntValue(value string) int {
	parsed, err := strconv.Atoi(value)
	if err != nil {
		panic(err)
	}
	return parsed
}

func parseBoolValue(value string) bool {
	parsed, err := strconv.ParseBool(value)
	if err != nil {
		panic(err)
	}
	return parsed
}

func LoadConnectionSettings(path string) MqttConnectionSettings {
	godotenv.Load(path)
	cs := MqttConnectionSettings{}
	envVars := make(map[string]string)

	// Check to see which env vars are set
	for i := 0; i < len(mqttSettingNames); i++ {
		name := mqttSettingNames[i]
		value := os.Getenv(name)
		// If var is not set, check if it has a default value
		if value == "" && defaults[name] != "" {
			value = defaults[name]
		}

		envVars[name] = value
	}

	// Based on which vars are set, construct MqttConnectionSettings
	cs.Hostname = envVars["MQTT_HOST_NAME"]
	cs.TcpPort = parseIntValue(envVars["MQTT_TCP_PORT"])
	cs.UseTls = parseBoolValue(envVars["MQTT_USE_TLS"])
	cs.CleanSession = parseBoolValue(envVars["MQTT_CLEAN_SESSION"])
	cs.KeepAlive = uint16(parseIntValue(envVars["MQTT_KEEP_ALIVE_IN_SECONDS"]))
	cs.ClientId = envVars["MQTT_CLIENT_ID"]
	cs.Username = envVars["MQTT_USERNAME"]
	cs.Password = envVars["MQTT_PASSWORD"]
	cs.CaFile = envVars["MQTT_CA_FILE"]
	cs.CertFile = envVars["MQTT_CERT_FILE"]
	cs.KeyFile = envVars["MQTT_KEY_FILE"]
	cs.KeyFilePassword = envVars["MQTT_KEY_FILE_PASSWORD"]

	return cs
}
