/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <mosquitto.h>
#include "setup.h"

#define TOPIC          "MQTT Examples"
#define PAYLOAD        "Hello World!"

/*
 * This sample sends five telemetry messages to the Broker. X509 self-certification is used.
 */
int main(  int argc, char *argv[] )
{
    struct mosquitto * mosq;
    int rc = 0;
    struct mosq_context * mqtt_context = calloc( 1, sizeof( struct mosq_context ) );
    struct connection_settings * cs = calloc(1, sizeof( struct connection_settings));

    mqtt_context->messagesSent = 0;

    mosq = initMQTT( false, true, argv[1], mqtt_context, cs );

    rc = mosquitto_connect( mosq, cs->broker_address, cs->broker_port, 60 );

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
        rc = mosquitto_publish( mosq, NULL, TOPIC, ( int ) strlen( PAYLOAD ), PAYLOAD, cs->qos, false );

        if( rc != MOSQ_ERR_SUCCESS )
        {
            printf( "Error publishing: %s\n", mosquitto_strerror( rc ) );
        }
    }

    mosquitto_loop_stop( mosq, true );

    mosquitto_lib_cleanup();
    free( mqtt_context );
    free( cs );
    return 0;
}
