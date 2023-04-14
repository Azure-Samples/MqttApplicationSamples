/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
#define TOPIC          "MQTT Examples"
#define QOS            1
#define TIMEOUT        10000L

/*
 * This sample receives five telemetry messages from the broker. X509 self-certification is used.
 */
int main( void )
{
    struct mosquitto * mosq;
    int rc = 0;
    struct mosq_context * context = calloc( 1, sizeof( struct mosq_context ) );

    context->messagesReceived = 0;

    mosq = initMQTT( true, false, true, context );
    rc = mosquitto_connect( mosq, ADDRESS, PORT, 60 );

    if( rc != MOSQ_ERR_SUCCESS )
    {
        mosquitto_destroy( mosq );
        printf( "Connection Error: %s\n", mosquitto_strerror( rc ) );
        return 1;
    }

    mosquitto_loop_forever( mosq, -1, 1 );

    mosquitto_lib_cleanup();
    free( context );
    return 0;
}
