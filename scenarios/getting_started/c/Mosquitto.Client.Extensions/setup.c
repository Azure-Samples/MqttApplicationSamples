// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <mosquitto.h>
#include "callbacks.h"
#include "setup.h"

// #define CLIENTID    "ExampleClientGettingStarted"


void setSubscribeCallbacks(struct mosquitto *mosq)
{
    mosquitto_connect_callback_set(mosq, on_connect_with_subscribe);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
    mosquitto_message_callback_set(mosq, on_message);
}

void setPublishCallbacks(struct mosquitto *mosq)
{
    mosquitto_publish_callback_set(mosq, on_publish);
}

struct mosquitto *initMQTT(bool subscribe, bool publish, bool useTLS, struct mosq_context *context)
{
    struct mosquitto *mosq = NULL;

    /* Required before calling other mosquitto functions */
	mosquitto_lib_init();

    /* Create a new client instance.
	* id = NULL -> ask the broker to generate a client id for us
	* clean session = true -> the broker should remove old sessions when we connect
	* obj = NULL -> we aren't passing any of our private data for callbacks
	*/
	mosq = mosquitto_new(NULL, true, context);
	if(mosq == NULL){
		printf("Error: Out of memory.\n");
		return NULL;
	}

    //callbacks    
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    if (subscribe)
    {
        setSubscribeCallbacks(mosq);
    }
    if (publish)
    {
        setPublishCallbacks(mosq);
        if (!subscribe)
        {
            mosquitto_connect_callback_set(mosq, on_connect);
        }
    }

    if (useTLS)
    {
        int rc = mosquitto_tls_set(mosq, "/home/vaavva/repos/Mosquitto/chain.pem", "/etc/ssl/certs", "/home/vaavva/repos/Mosquitto/vehicle01.pem", "/home/vaavva/repos/Mosquitto/vehicle01.key", NULL);
	    if(rc != MOSQ_ERR_SUCCESS){
			mosquitto_destroy(mosq);
			printf("TLS Error: %s\n", mosquitto_strerror(rc));
			return NULL;
		}
    }

    return mosq;
}

