/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#define USE_SSL

#ifdef USE_SSL
    #define ADDRESS    "localhost"
    #define PORT       8883
#else
    #define ADDRESS    "localhost"
    #define PORT       1883
#endif

#define CLIENTID       "ExampleClientGettingStarted"
#define TOPIC          "MQTT Examples"
#define PAYLOAD        "Hello World!"
#define QOS            1
#define TIMEOUT        10000L

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect( struct mosquitto * mosq,
                 void * obj,
                 int reason_code );

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect_with_subscribe( struct mosquitto * mosq,
                                void * obj,
                                int reason_code );

void on_disconnect( struct mosquitto * mosq,
                    void * obj,
                    int rc );

/* Callback called when the broker sends a SUBACK in response to a SUBSCRIBE. */
void on_subscribe( struct mosquitto * mosq,
                   void * obj,
                   int mid,
                   int qos_count,
                   const int * granted_qos );


/* Callback called when the client receives a message. */
void on_message( struct mosquitto * mosq,
                 void * obj,
                 const struct mosquitto_message * msg );

/* Callback called when the client knows to the best of its abilities that a
 * PUBLISH has been successfully sent. For QoS 0 this means the message has
 * been completely written to the operating system. For QoS 1 this means we
 * have received a PUBACK from the broker. For QoS 2 this means we have
 * received a PUBCOMP from the broker. */
void on_publish( struct mosquitto * mosq,
                 void * obj,
                 int mid );
