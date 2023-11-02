package main

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"log"
	"net/url"
	"os"
	"os/signal"
	"syscall"

	"github.com/eclipse/paho.golang/autopaho"
	"github.com/eclipse/paho.golang/paho"
	//"github.com/joho/godotenv"
)

func main() {
	fmt.Println("hello from go!")

	// Load certificates
	cert, err := tls.LoadX509KeyPair("../sample_client.pem", "../sample_client.key")
	if err != nil {
		log.Fatal(err)
	}

	caCert, _ := os.ReadFile("../chain.pem")
	caCertPool := x509.NewCertPool()
	caCertPool.AppendCertsFromPEM(caCert)

	u, err := url.Parse("mqtt://npatilsen-eventgrid.westus2-1.ts.eventgrid.azure.net:8883")
	if err != nil {
		panic(err)
	}

	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()

	cfg := &tls.Config{
		Certificates: []tls.Certificate{cert},
		RootCAs:      caCertPool,
	}

	clientConfig := autopaho.ClientConfig{
		BrokerUrls:     []*url.URL{u},
		TlsCfg:         cfg,
		KeepAlive:      20,
		ConnectTimeout: 10,
		OnConnectionUp: func(cm *autopaho.ConnectionManager, connAck *paho.Connack) {
			fmt.Println("mqtt connection up")
			if _, err := cm.Subscribe(context.Background(), &paho.Subscribe{
				Subscriptions: []paho.SubscribeOptions{
					{Topic: "sample/+", QoS: 1},
				},
			}); err != nil {
				fmt.Printf("failed to subscribe (%s). This is likely to mean no messages will be received!\n", err)
			}
			fmt.Println("mqtt subscription made")
		},
		OnConnectError: func(err error) { fmt.Printf("error whilst attempting connection: %s\n", err) },
		ClientConfig: paho.ClientConfig{
			ClientID: "sample_client",
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

	fmt.Println("attempting to connect")
	c, err := autopaho.NewConnection(ctx, clientConfig) // starts process; will reconnect until context cancelled
	if err != nil {
		panic(err)
	}
	// Wait for the connection to come up
	if err = c.AwaitConnection(ctx); err != nil {
		panic(err)
	}

	fmt.Println("connection successful!")

	// Publish
	if _, err = c.Publish(ctx, &paho.Publish{
		QoS:     1,
		Topic:   "sample/+",
		Payload: []byte("TestMessage"),
	}); err != nil {
		panic(err)
	}
	fmt.Println("published!!!")

	<-ctx.Done() // Wait for user to trigger exit
	fmt.Println("signal caught - exiting")
}
