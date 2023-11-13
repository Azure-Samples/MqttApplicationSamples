package main

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"log"
	"net"
	"os"
	"os/signal"
	"syscall"

	"mqttapplicationsamples/ConnectionSettings"

	"github.com/eclipse/paho.golang/paho"
)

func getTlsConnection(certFile string, keyFile string, caFile string, hostname string, TcpPort int) *tls.Conn {
	fmt.Println("Loading certificates")
	cert, err := tls.LoadX509KeyPair(fmt.Sprintf("../%s", certFile), fmt.Sprintf("../%s", keyFile))
	if err != nil {
		log.Fatal(err)
	}

	cfg := &tls.Config{
		Certificates: []tls.Certificate{cert},
	}

	if caFile != "" {
		ca, err := os.ReadFile(fmt.Sprintf("../%s", caFile))
		if err != nil {
			panic(err)
		}

		caCertPool := x509.NewCertPool()
		caCertPool.AppendCertsFromPEM(ca)
		cfg.RootCAs = caCertPool
	}

	conn, err := tls.Dial("tcp", fmt.Sprintf("%s:%d", hostname, TcpPort), cfg)
	if err != nil {
		panic(err)
	}

	return conn
}

func main() {
	// Load connection settings
	var cs ConnectionSettings.MqttConnectionSettings = ConnectionSettings.LoadConnectionSettings("../.env")

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
		c.Conn = getTlsConnection(cs.CertFile, cs.KeyFile, cs.CaFile, cs.Hostname, cs.TcpPort)
	} else {
		conn, err := net.Dial("tcp", cs.Hostname)
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

	fmt.Println("Attempting to connect")
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
