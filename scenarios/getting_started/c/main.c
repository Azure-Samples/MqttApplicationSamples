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
int main( int argc,
          char * argv[] )
{
    struct mosquitto * mosq;
    int rc = 0;
    struct connection_settings * cs = calloc( 1, sizeof( struct connection_settings ) );

    cs->sub_topic = "sample/+";

    mosq = initMQTT( true, argv[ 1 ], cs );
    rc = mosquitto_connect_bind_v5( mosq, cs->hostname, cs->tcp_port, cs->keep_alive_in_seconds, NULL, NULL );

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

    while( 1 )
    {
        rc = mosquitto_publish_v5( mosq, NULL, "sample/topic1", ( int ) strlen( PAYLOAD ), PAYLOAD, cs->qos, false, NULL );

        if( rc != MOSQ_ERR_SUCCESS )
        {
            printf( "Error publishing: %s\n", mosquitto_strerror( rc ) );
        }

        sleep( 5 );
    }

    mosquitto_loop_stop( mosq, false );

    mosquitto_lib_cleanup();
    free( cs );
    return 0;
}
