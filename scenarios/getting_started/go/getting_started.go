package main

import (
	"context"
	"crypto/tls"
	"fmt"
	"log"
	"net/url"
	"os"
	"os/signal"
	"syscall"

	"github.com/eclipse/paho.golang/autopaho"
	"github.com/eclipse/paho.golang/paho"
	"github.com/joho/godotenv"
)

func main() {
	// Load environment variables
	godotenv.Load("../.env")
	hostname := os.Getenv("MQTT_HOST_NAME")
	username := os.Getenv("MQTT_USERNAME")
	clientId := os.Getenv("MQTT_CLIENT_ID")
	certFile := os.Getenv("MQTT_CERT_FILE")
	keyFile := os.Getenv("MQTT_KEY_FILE")
	topic := "sample/+"

	// Load certificates
	cert, err := tls.LoadX509KeyPair(fmt.Sprintf("../%s", certFile), fmt.Sprintf("../%s", keyFile))
	if err != nil {
		log.Fatal(err)
	}

	u, err := url.Parse(fmt.Sprintf("mqtts://%s:8883", hostname))
	if err != nil {
		panic(err)
	}

	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()

	cfg := &tls.Config{
		Certificates: []tls.Certificate{cert},
	}

	clientConfig := autopaho.ClientConfig{
		BrokerUrls: []*url.URL{u},
		TlsCfg:     cfg,
		KeepAlive:  30,
		OnConnectionUp: func(cm *autopaho.ConnectionManager, connAck *paho.Connack) {
			fmt.Println("mqtt connection up")
			if _, err := cm.Subscribe(context.Background(), &paho.Subscribe{
				Subscriptions: []paho.SubscribeOptions{
					{Topic: topic, QoS: 1},
				},
			}); err != nil {
				fmt.Printf("failed to subscribe (%s). This is likely to mean no messages will be received!\n", err)
			}
			fmt.Println("mqtt subscription made")
		},
		OnConnectError: func(err error) { fmt.Printf("error whilst attempting connection: %s\n", err) },
		ClientConfig: paho.ClientConfig{
			ClientID: clientId,
			Router: paho.NewSingleHandlerRouter(func(m *paho.Publish) {
				fmt.Printf("received message on topic %s; body: %s (retain: %t)\n", m.Topic, m.Payload, m.Retain)
			}),
			OnClientError: func(err error) { fmt.Printf("server requested disconnect: %s\n", err) },
			OnServerDisconnect: func(d *paho.Disconnect) {
				if d.Properties != nil {
					fmt.Printf("server requested disconnect: %s\n", d.Properties.ReasonString)
				} else {
					fmt.Printf("server requested disconnect; reason code: %d\n", d.ReasonCode)
				}
			},
		},
	}

	clientConfig.SetUsernamePassword(username, nil)

	fmt.Println("Attempting to connect")
	c, err := autopaho.NewConnection(ctx, clientConfig) // starts process; will reconnect until context cancelled
	if err != nil {
		panic(err)
	}
	// Wait for the connection to come up
	if err = c.AwaitConnection(ctx); err != nil {
		panic(err)
	}

	fmt.Println("Connection established")

	// Publish
	if _, err = c.Publish(ctx, &paho.Publish{
		QoS:     1,
		Topic:   topic,
		Payload: []byte("Hello, from Go!"),
	}); err != nil {
		panic(err)
	}

	<-ctx.Done() // Wait for user to trigger exit
	fmt.Println("signal caught - exiting")
}
