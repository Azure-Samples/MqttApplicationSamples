/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <mosquitto.h>
#include "callbacks.h"
#include "setup.h"

static struct connection_settings cs;

void setConnectionSettings()
{
    cs.broker_address = getenv( "BROKER_ADDRESS" );
    cs.broker_port = atoi( getenv( "BROKER_PORT" ) );
    cs.client_id = getenv( "CLIENT_ID" );
    cs.ca_file = getenv( "CA_FILE" );
    cs.ca_path = getenv( "CA_PATH" );
    cs.cert_file = getenv( "CERT_FILE" );
    cs.key_file = getenv( "KEY_FILE" );
    cs.qos = atoi( getenv( "QOS" ) );
    cs.keep_alive_in_seconds = atoi( getenv( "KEEP_ALIVE_IN_SECONDS" ) );
    cs.use_TLS = true;
    cs.mqtt_version = MQTT_PROTOCOL_V311;
}

void setSubscribeCallbacks( struct mosquitto * mosq )
{
    mosquitto_connect_callback_set( mosq, on_connect_with_subscribe );
    mosquitto_subscribe_callback_set( mosq, on_subscribe );
    mosquitto_message_callback_set( mosq, on_message );
}

void setPublishCallbacks( struct mosquitto * mosq )
{
    mosquitto_publish_callback_set( mosq, on_publish );
}

struct mosquitto * initMQTT( bool subscribe,
                             bool publish,
                             bool useTLS,
                             struct mosq_context * context )
{
    setConnectionSettings();
    struct mosquitto * mosq = NULL;

    /* Required before calling other mosquitto functions */
    mosquitto_lib_init();

    /* Create a new client instance.
     * id = NULL -> ask the broker to generate a client id for us
     * clean session = true -> the broker should remove old sessions when we connect
     * obj = NULL -> we aren't passing any of our private data for callbacks
     */
    mosq = mosquitto_new( cs.client_id, true, context );

    if( mosq == NULL )
    {
        printf( "Error: Out of memory.\n" );
        return NULL;
    }

    /*callbacks */
    mosquitto_disconnect_callback_set( mosq, on_disconnect );

    if( subscribe )
    {
        setSubscribeCallbacks( mosq );
    }

    if( publish )
    {
        setPublishCallbacks( mosq );

        if( !subscribe )
        {
            mosquitto_connect_callback_set( mosq, on_connect );
        }
    }

    if( useTLS )
    {
        int rc = mosquitto_tls_set( mosq, cs.ca_file, cs.ca_path, cs.cert_file, cs.key_file, NULL );

        if( rc != MOSQ_ERR_SUCCESS )
        {
            mosquitto_destroy( mosq );
            printf( "TLS Error: %s\n", mosquitto_strerror( rc ) );
            return NULL;
        }
    }

    return mosq;
}
