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
    struct connection_settings * cs = calloc( 1, sizeof( struct connection_settings ) );

    cs->sub_topic = "vehicles/+/position";

    mosq = initMQTT( false, argv[ 1 ], cs );
    rc = mosquitto_connect_bind_v5( mosq, cs->hostname, cs->tcp_port, cs->keep_alive_in_seconds, NULL, NULL );

    if( rc != MOSQ_ERR_SUCCESS )
    {
        mosquitto_destroy( mosq );
        printf( "Connection Error: %s\n", mosquitto_strerror( rc ) );
        return 1;
    }

    mosquitto_loop_forever( mosq, -1, 1 );

    mosquitto_lib_cleanup();
    free( cs );
    return 0;
}
