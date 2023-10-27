package main

import (
	"bufio"
	"context"
	"flag"
	"fmt"
	"io"
	"log"
	"net"
	"os"
	"os/signal"
	"syscall"

	"github.com/eclipse/paho.golang/paho"
	"github.com/joho/godotenv"
)

func main() {
	fmt.Println("hello from go!")
	stdin := bufio.NewReader(os.Stdin)

	// Load connection settings via environment variables and set connection parameters
	godotenv.Load("../.env")

	server := flag.String("server", fmt.Sprintf("%s:%s", os.Getenv("MQTT_HOST_NAME"), "8883"), "The full URL of the MQTT server to connect to")
	topic := "sample/+"
	qos := flag.Int("qos", 0, "The QoS to send the messages at")
	retained := flag.Bool("retained", false, "Are the messages sent with the retained flag")
	clientId := os.Getenv("MQTT_CLIENT_ID")
	username := os.Getenv("MQTT_USERNAME")
	flag.Parse()

	conn, err := net.Dial("tcp", *server)
	if err != nil {
		log.Fatalf("Failed to connect to %s: %s", *server, err)
	}

	c := paho.NewClient(paho.ClientConfig{
		Conn: conn,
	})

	cp := &paho.Connect{
		KeepAlive:  30,
		ClientID:   clientId,
		CleanStart: true,
		Username:   username,
		Password:   nil,
	}

	cp.UsernameFlag = true

	log.Println(cp.UsernameFlag)

	ca, err := c.Connect(context.Background(), cp)
	if err != nil {
		log.Fatalln(err)
	}
	if ca.ReasonCode != 0 {
		log.Fatalf("Failed to connect to %s : %d - %s", *server, ca.ReasonCode, ca.Properties.ReasonString)
	}

	fmt.Printf("Connected to %s\n", *server)

	ic := make(chan os.Signal, 1)
	signal.Notify(ic, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-ic
		fmt.Println("signal received, exiting")
		if c != nil {
			d := &paho.Disconnect{ReasonCode: 0}
			c.Disconnect(d)
		}
		os.Exit(0)
	}()

	for {
		message, err := stdin.ReadString('\n')
		if err == io.EOF {
			os.Exit(0)
		}

		if _, err = c.Publish(context.Background(), &paho.Publish{
			Topic:   topic,
			QoS:     byte(*qos),
			Retain:  *retained,
			Payload: []byte(message),
		}); err != nil {
			log.Println("error sending message:", err)
			continue
		}
		log.Println("sent")
	}

}

// do we have pub and sub?
// disconnect
