/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <stdio.h>

#include <mosquitto.h>
#include "mqtt_callbacks.h"

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect( struct mosquitto * mosq,
                 void * obj,
                 int reason_code,
                 int flags,
                 const mosquitto_property * props )
{
    /* TODO: pass mqtt version in obj and use mosquitto_reason_string if mqtt5 */

    /* Print out the connection result. mosquitto_connack_string() produces an
     * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
     * clients is mosquitto_reason_string().
     */
    printf( "on_connect: %s\n", mosquitto_connack_string( reason_code ) );

    if( reason_code != 0 )
    {
        /* If the connection fails for any reason, we don't want to keep on
         * retrying in this example, so disconnect. Without this, the client
         * will attempt to reconnect. */
        mosquitto_disconnect_v5( mosq, reason_code, props ); /* TODO: double check this is the reason_code that should be passed in */
    }
}

/* Callback called when the client receives a CONNACK message from the broker and we want to subscribe on connect. */
void on_connect_with_subscribe( struct mosquitto * mosq,
                                void * obj,
                                int reason_code,
                                int flags,
                                const mosquitto_property * props )
{
    on_connect( mosq, obj, reason_code, flags, props );

    int result;

    /* Making subscriptions in the on_connect() callback means that if the
     * connection drops and is automatically resumed by the client, then the
     * subscriptions will be recreated when the client reconnects. */
    result = mosquitto_subscribe_v5( mosq, NULL, getenv( "SUB_TOPIC" ), atoi( getenv( "QOS" ) ), 0, NULL );

    if( result != MOSQ_ERR_SUCCESS )
    {
        printf( "Error subscribing: %s\n", mosquitto_strerror( result ) );
        /* We might as well disconnect if we were unable to subscribe */
        mosquitto_disconnect_v5( mosq, reason_code, props );
    }
}

/* Callback called when the broker has received the DISCONNECT command and has disconnected the client. */
void on_disconnect( struct mosquitto * mosq,
                    void * obj,
                    int rc,
                    const mosquitto_property * props )
{
    printf( "on_disconnect: reason=%s\n", mosquitto_strerror( rc ) );
}

/* Callback called when the broker sends a SUBACK in response to a SUBSCRIBE. */
void on_subscribe( struct mosquitto * mosq,
                   void * obj,
                   int mid,
                   int qos_count,
                   const int * granted_qos,
                   const mosquitto_property * props )
{
    printf( "on_subscribe: Subscribed with mid %d; %d topics.\n", mid, qos_count );

    /* In this example we only subscribe to a single topic at once, but a
     * SUBSCRIBE can contain many topics at once, so this is one way to check
     * them all. */
    for( int i = 0; i < qos_count; i++ )
    {
        printf( "\tQoS %d\n", granted_qos[ i ] );
    }
}


/* Callback called when the client receives a message. */
void on_message( struct mosquitto * mosq,
                 void * obj,
                 const struct mosquitto_message * msg,
                 const mosquitto_property * props )
{
    /* This blindly prints the payload, but the payload can be anything so take care. */
    printf( "on_message: Topic: %s; QOS: %d; Payload: %s\n", msg->topic, msg->qos, ( char * ) msg->payload );
}

/* Callback called when the client knows to the best of its abilities that a
 * PUBLISH has been successfully sent. For QoS 0 this means the message has
 * been completely written to the operating system. For QoS 1 this means we
 * have received a PUBACK from the broker. For QoS 2 this means we have
 * received a PUBCOMP from the broker. */
void on_publish( struct mosquitto * mosq,
                 void * obj,
                 int mid,
                 int reason_code,
                 const mosquitto_property * props )
{
    printf( "on_publish: Message with mid %d has been published.\n", mid );
}
