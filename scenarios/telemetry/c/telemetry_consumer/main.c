/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <mosquitto.h>
#include "setup.h"

/*
 * This sample receives five telemetry messages from the broker. X509 self-certification is used.
 */
int main( int argc,
          char * argv[] )
{
    struct mosquitto * mosq;
    int rc = 0;
    struct mosq_context * mqtt_context = calloc( 1, sizeof( struct mosq_context ) );
    struct connection_settings * cs = calloc( 1, sizeof( struct connection_settings ) );

    cs->sub_topic = "vehicles/+/position";
    mqtt_context->messagesReceived = 0;

    mosq = initMQTT( false, argv[ 1 ], mqtt_context, cs );
    rc = mosquitto_connect( mosq, cs->hostname, cs->tcp_port, cs->keep_alive_in_seconds );

    if( rc != MOSQ_ERR_SUCCESS )
    {
        mosquitto_destroy( mosq );
        printf( "Connection Error: %s\n", mosquitto_strerror( rc ) );
        return 1;
    }

    mosquitto_loop_forever( mosq, -1, 1 );

    mosquitto_lib_cleanup();
    free( mqtt_context );
    free( cs );
    return 0;
}