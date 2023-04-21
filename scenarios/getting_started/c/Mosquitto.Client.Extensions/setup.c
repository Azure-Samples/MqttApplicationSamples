/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <mosquitto.h>
#include "callbacks.h"
#include "setup.h"

void read_env_file( char * filePath )
{
    /* TODO: think about whether this is an issue if we specify the env variables in launch.json, but have a .env that then overwrites them */
    if( filePath == NULL )
    {
        filePath = ".env";
    }

    FILE * fptr = fopen( filePath, "r" );

    if( fptr != NULL )
    {
        char envString[ 300 ];
        char envName[ 30 ];
        char envValue[ 256 ];

        while( fscanf( fptr, "%s", envString ) == 1 )
        {
            char * envName = strtok( envString, "=" );
            char * envValue = strtok( NULL, "=\"" );
            printf( "Setting %s = %s\n", envName, envValue );
            setenv( envName, envValue, 1 );
        }

        fclose( fptr );
    }
    else
    {
        printf( "Cannot open env file, will try to use environment variables. \n" );
    }
}

void setConnectionSettings( struct connection_settings * cs )
{
    cs->hostname = getenv( "HOSTNAME" );
    cs->tcp_port = atoi( getenv( "TCP_PORT" ) ? : "8883" );
    cs->client_id = getenv( "CLIENT_ID" );
    cs->ca_file = getenv( "CA_FILE" );
    /* TODO: this might not work because we could keep an env var from a previous session */
    cs->ca_path = getenv( "CA_PATH" ) ? : cs->ca_file ? NULL : "/etc/ssl/certs";
    cs->cert_file = getenv( "CERT_FILE" );
    cs->key_file = getenv( "KEY_FILE" );
    cs->key_file_password = getenv( "KEY_FILE_PASSWORD" );
    cs->qos = atoi( getenv( "QOS" ) ? : "1" );
    cs->keep_alive_in_seconds = atoi( getenv( "KEEP_ALIVE_IN_SECONDS" ) ? : "30" );
    char * use_TLS = getenv( "USE_TLS" );
    cs->use_TLS = ( use_TLS != NULL && strcmp( use_TLS, "false" ) == 0 ) ? false : true; /* TODO: figure out "cat" case */
    cs->mqtt_version = atoi( getenv( "MQTT_VERSION" ) ? : "4" );
    cs->username = getenv( "USERNAME" );
    cs->password = getenv( "PASSWORD" );
    cs->clean_session = atol( getenv( "CLEAN_SESSION" ) ? : "true" ); /* TODO: figure out "cat" case */
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

struct mosquitto * initMQTT( bool publish,
                             char * envFile,
                             struct mosq_context * context,
                             struct connection_settings * cs )
{
    bool subscribe = false;

    if( cs->sub_topic )
    {
        setenv( "SUB_TOPIC", cs->sub_topic, 1 );
        subscribe = true;
    }

    read_env_file( envFile );

    setConnectionSettings( cs );
    struct mosquitto * mosq = NULL;

    /* Required before calling other mosquitto functions */
    mosquitto_lib_init();

    /* Create a new client instance.
     * id = NULL -> ask the broker to generate a client id for us
     * clean session = true -> the broker should remove old sessions when we connect
     * obj = NULL -> we aren't passing any of our private data for callbacks
     */
    mosq = mosquitto_new( cs->client_id, cs->clean_session, context );

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

    int rc;

    if( cs->username )
    {
        rc = mosquitto_username_pw_set( mosq, cs->username, cs->password );

        if( rc != MOSQ_ERR_SUCCESS )
        {
            mosquitto_destroy( mosq );
            printf( "Error setting username/password: %s\n", mosquitto_strerror( rc ) );
            return NULL;
        }
    }

    if( cs->use_TLS )
    {
        rc = mosquitto_tls_set( mosq, cs->ca_file, cs->ca_path, cs->cert_file, cs->key_file, NULL );

        if( rc != MOSQ_ERR_SUCCESS )
        {
            mosquitto_destroy( mosq );
            printf( "TLS Error: %s\n", mosquitto_strerror( rc ) );
            return NULL;
        }
    }

    return mosq;
}