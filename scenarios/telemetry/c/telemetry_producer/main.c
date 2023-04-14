/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <mosquitto.h>
#include "setup.h"

#define USE_SSL

#ifdef USE_SSL
    #define ADDRESS    "localhost"
    #define PORT       8883
#else
    #define ADDRESS    "localhost"
    #define PORT       1883
#endif

#define CLIENTID       "ExampleClientPub"
#define TOPIC          "MQTT Examples"
#define PAYLOAD        "Hello World!"
#define QOS            1

/*
 * This sample sends five telemetry messages to the Broker. X509 self-certification is used.
 */
int main( void )
{
    struct mosquitto * mosq;
    int rc = 0;
    struct mosq_context * context = calloc( 1, sizeof( struct mosq_context ) );

    context->messagesSent = 0;

    mosq = initMQTT( false, true, true, context );

    rc = mosquitto_connect( mosq, ADDRESS, PORT, 60 );

    if( rc != MOSQ_ERR_SUCCESS )
    {
        mosquitto_destroy( mosq );
        printf( "Connection Error: %s\n", mosquitto_strerror( rc ) );
        return 1;
    }

    rc = mosquitto_loop_start( mosq );

    if( rc != MOSQ_ERR_SUCCESS )
    {
        mosquitto_destroy( mosq );
        printf( "loop Error: %s\n", mosquitto_strerror( rc ) );
        return 1;
    }

    while( context->messagesSent < 5 )
    {
        sleep( 1 );
        rc = mosquitto_publish( mosq, NULL, TOPIC, ( int ) strlen( PAYLOAD ), PAYLOAD, QOS, false );

        if( rc != MOSQ_ERR_SUCCESS )
        {
            printf( "Error publishing: %s\n", mosquitto_strerror( rc ) );
        }
    }

    mosquitto_loop_stop( mosq, true );

    mosquitto_lib_cleanup();
    free( context );
    return 0;
}
