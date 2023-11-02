package main

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"
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
	//stdin := bufio.NewReader(os.Stdin)

	/*
		// Load connection settings via environment variables and set connection parameters
		godotenv.Load("../.env")

		server := flag.String("server", fmt.Sprintf("%s:%s", os.Getenv("MQTT_HOST_NAME"), "8883"), "The full URL of the MQTT server to connect to")
		topic := "sample/+"
		qos := flag.Int("qos", 0, "The QoS to send the messages at")
		//retained := flag.Bool("retained", false, "Are the messages sent with the retained flag")
		clientId := os.Getenv("MQTT_CLIENT_ID")
		//username := os.Getenv("MQTT_USERNAME")
		//password := "YPC/ETXxvltLdv+hHupUCczQSc8Te4wtwSHx1P49Qxs="
		flag.Parse()*/

	// Load certificates
	cert, err := tls.LoadX509KeyPair("../sample_client.pem", "../sample_client.key")
	if err != nil {
		log.Fatal(err)
	}

	caCert, _ := ioutil.ReadFile("../chain.pem")
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
		ClientCAs:    caCertPool,
	}

	// is this needed?
	/*
			listener, err := tls.Listen("tcp", "npatilsen-eventgrid.westus2-1.ts.eventgrid.azure.net:8883", cfg)
			if err != nil {
				log.Fatal(err)
			}
			_ = listener


		fmt.Println("dialing eventgrid")
		conn, err := net.Dial("tcp", "npatilsen-eventgrid.westus2-1.ts.eventgrid.azure.net:8883")
		if err != nil {
			log.Fatalf("Failed to connect to %s: %s", "npatilsen-eventgrid.westus2-1.ts.eventgrid.azure.net:8883", err)
		}
	*/

	clientConfig := autopaho.ClientConfig{
		BrokerUrls: []*url.URL{u},
		TlsCfg:     cfg,
		KeepAlive:  20,
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

/*
		c := paho.NewClient(clientConfig)

		properties := paho.ConnectProperties{
			AuthData:   nil,
			AuthMethod: "Intermediate01",
		}

		cp := &paho.Connect{
			KeepAlive:  30,
			ClientID:   clientId,
			CleanStart: true,
			Properties: &properties,
			Username:   username,
			Password:   []byte(password),
		}


	cp.UsernameFlag = true

	log.Println(cp.UsernameFlag)

	// TODO - fix
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
*/
