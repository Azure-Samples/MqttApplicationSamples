/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <stdio.h>

#include <mosquitto.h>
#include "mqtt_setup.h"

/*
 * This sample receives telemetry messages from the broker. X509 certification is used.
 */
int main( int argc,
          char * argv[] )
{
    struct mosquitto * mosq;
    int result = 0;
    mqtt_client_connection_settings * connection_settings = calloc( 1, sizeof( mqtt_client_connection_settings ) );

    connection_settings->sub_topic = "vehicles/+/position";

    mosq = mqtt_client_init( false, argv[ 1 ], connection_settings );
    result = mosquitto_connect_bind_v5( mosq, connection_settings->hostname, connection_settings->tcp_port, connection_settings->keep_alive_in_seconds, NULL, NULL );

    if( result != MOSQ_ERR_SUCCESS )
    {
        mosquitto_destroy( mosq );
        printf( "Connection Error: %s\n", mosquitto_strerror( result ) );
        return 1;
    }

    mosquitto_loop_forever( mosq, -1, 1 );

    mosquitto_destroy( mosq );
    mosquitto_lib_cleanup();
    free( connection_settings );
    return 0;
}
