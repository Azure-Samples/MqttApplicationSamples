/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <mosquitto.h>
#include "setup.h"

#define PAYLOAD    "Hello World!" /* TODO: position */

/*
 * This sample sends telemetry messages to the Broker. X509 certification is used.
 */
int main( int argc,
          char * argv[] )
{
    struct mosquitto * mosq;
    int rc = 0;
    struct connection_settings * cs = calloc( 1, sizeof( struct connection_settings ) );

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

    char topic[ strlen( cs->client_id ) + 17 ];
    sprintf( topic, "vehicles/%s/position", cs->client_id );

    while( 1 )
    {
        rc = mosquitto_publish_v5( mosq, NULL, topic, ( int ) strlen( PAYLOAD ), PAYLOAD, cs->qos, false, NULL );

        if( rc != MOSQ_ERR_SUCCESS )
        {
            printf( "Error publishing: %s\n", mosquitto_strerror( rc ) );
        }

        sleep( 5 );
    }

    mosquitto_loop_stop( mosq, true );

    mosquitto_destroy( mosq );
    mosquitto_lib_cleanup();
    free( cs );
    return 0;
}
