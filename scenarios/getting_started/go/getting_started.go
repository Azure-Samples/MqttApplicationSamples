package main

import (
	"fmt"
	"os"

	mqtt "github.com/eclipse/paho.golang/paho"
	"github.com/joho/godotenv"
)

func main() {
	fmt.Println("hello from go!")

	// Load connection settings via environment variables
	godotenv.Load("../.env")

	clientConfig := mqtt.ClientConfig{
		ClientID: os.Getenv("MQTT_CLIENT_ID"),
	}

	connectPacket := mqtt.Connect{
		Username: os.Getenv("MQTT_USERNAME"),
	}

	client := mqtt.NewClient(clientConfig)
	client.Connect()
}

// initialize client

// connect

// subscribe/suback

// publish/puback

// disconnect
