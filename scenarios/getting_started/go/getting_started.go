package main

import (
	"context"
	"fmt"
	"log"
	"mqttapplicationsamples/ConnectionSettings"
	"net"
	"os"
	"os/signal"
	"syscall"

	"github.com/eclipse/paho.golang/paho"
)

func main() {
	// Load connection settings
	if len(os.Args) <= 1 {
		panic("Please specify path to environment variables")
	}

	var cs ConnectionSettings.MqttConnectionSettings = ConnectionSettings.LoadConnectionSettings(os.Args[1])

	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()
	fmt.Println("Creating Paho client")
	c := paho.NewClient(paho.ClientConfig{
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
	})

	if cs.UseTls {
		c.Conn = ConnectionSettings.GetTlsConnection(cs)
	} else {
		conn, err := net.Dial("tcp", fmt.Sprintf("%s:%d", cs.Hostname, cs.TcpPort))
		if err != nil {
			panic(err)
		}
		c.Conn = conn
	}

	cp := &paho.Connect{
		KeepAlive:  cs.KeepAlive,
		ClientID:   cs.ClientId,
		CleanStart: cs.CleanSession,
	}

	if cs.Username != "" {
		cp.Username = cs.Username
		cp.UsernameFlag = true
	}

	if cs.Password != "" {
		cp.Password = []byte(cs.Password)
		cp.PasswordFlag = true
	}

	fmt.Printf("Attempting to connect to %s\n", cs.Hostname)
	ca, err := c.Connect(ctx, cp)
	if err != nil {
		log.Fatalln(err)
	}
	if ca.ReasonCode != 0 {
		log.Fatalf("Failed to connect to %s : %d - %s", cs.Hostname, ca.ReasonCode, ca.Properties.ReasonString)
	}

	fmt.Printf("Connection successful")
	c.Subscribe(ctx, &paho.Subscribe{
		Subscriptions: []paho.SubscribeOptions{
			{Topic: "sample/+", QoS: byte(1)},
		},
	})

	c.Publish(context.Background(), &paho.Publish{
		Topic:   "sample/topic1",
		QoS:     byte(1),
		Retain:  false,
		Payload: []byte("hello world"),
	})

	<-ctx.Done() // Wait for user to trigger exit
	fmt.Println("signal caught - exiting")
}
