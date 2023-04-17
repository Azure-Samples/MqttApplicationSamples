/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <mosquitto.h>
#include "setup.h"

#define PAYLOAD    "Hello World!"


/*
 * This sample sends and receives five telemetry messages to/from the Broker. X509 self-certification is used.
 */
int main( int argc, char *argv[] )
{
    struct mosquitto * mosq;
    int rc = 0;
    struct mosq_context * mqtt_context = calloc( 1, sizeof( struct mosq_context ) );
    struct connection_settings * cs = calloc(1, sizeof( struct connection_settings));

    cs->sub_topic = "sample/+";
    mqtt_context->messagesSent = 0;
    mqtt_context->messagesReceived = 0;
    mosq = initMQTT( true, true, argv[1], mqtt_context, cs );
    rc = mosquitto_connect( mosq, cs->broker_address, cs->broker_port, cs->keep_alive_in_seconds );

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

    while( mqtt_context->messagesSent < 5 )
    {
        sleep( 1 );
        rc = mosquitto_publish( mosq, NULL, "sample/topic1", ( int ) strlen( PAYLOAD ), PAYLOAD, cs->qos, false );

        if( rc != MOSQ_ERR_SUCCESS )
        {
            printf( "Error publishing: %s\n", mosquitto_strerror( rc ) );
        }
    }

    mosquitto_loop_stop( mosq, false );

    mosquitto_lib_cleanup();
    free( mqtt_context );
    free( cs );
    return 0;
}
