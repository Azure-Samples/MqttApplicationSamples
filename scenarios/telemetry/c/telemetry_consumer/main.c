// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <mosquitto.h>

#define USE_SSL

#ifdef USE_SSL
  #define ADDRESS     "localhost"
  #define PORT        8883
#else
  #define ADDRESS     "localhost"
  #define PORT        1883
#endif
#define CLIENTID    "ExampleClientSub"
#define TOPIC       "MQTT Examples"
#define QOS         1
#define TIMEOUT     10000L

static int msgCount = 0;

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	int rc;

	/* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		mosquitto_disconnect(mosq);
	}

	/* Making subscriptions in the on_connect() callback means that if the
	 * connection drops and is automatically resumed by the client, then the
	 * subscriptions will be recreated when the client reconnects. */
	rc = mosquitto_subscribe(mosq, NULL, TOPIC, QOS);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
		/* We might as well disconnect if we were unable to subscribe */
		mosquitto_disconnect(mosq);
	}

}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
  printf("DISCONNECT reason=%d\n", rc);
}


/* Callback called when the broker sends a SUBACK in response to a SUBSCRIBE. */
void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	printf("Subscribed with mid %d; %d topics.\n", mid, qos_count);

	/* In this example we only subscribe to a single topic at once, but a
	 * SUBSCRIBE can contain many topics at once, so this is one way to check
	 * them all. */
	for(int i=0; i < qos_count; i++){
		printf("QoS %d\n", granted_qos[i]);
	}
}


/* Callback called when the client receives a message. */
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	/* This blindly prints the payload, but the payload can be anything so take care. */
	printf("%s: %d: %s\n", msg->topic, msg->qos, (char *)msg->payload);
	msgCount++;
	if (msgCount > 4)
	{
		mosquitto_disconnect(mosq);
	}
}


/*
 * This sample receives five telemetry messages from the broker. X509 self-certification is used.
 */
int main(void)
{

  	struct mosquitto *mosq;
	int rc;

	/* Required before calling other mosquitto functions */
	mosquitto_lib_init();

	/* Create a new client instance.
	* id = NULL -> ask the broker to generate a client id for us
	* clean session = true -> the broker should remove old sessions when we connect
	* obj = NULL -> we aren't passing any of our private data for callbacks
	*/
	mosq = mosquitto_new(CLIENTID, true, NULL);
	if(mosq == NULL){
		printf("Error: Out of memory.\n");
		return 1;
	}

	/* Configure callbacks. This should be done before connecting ideally. */
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_disconnect_callback_set(mosq, on_disconnect);
	mosquitto_subscribe_callback_set(mosq, on_subscribe);
	mosquitto_message_callback_set(mosq, on_message);

#ifdef USE_SSL
	rc = mosquitto_tls_set(mosq, "/home/vaavva/repos/Mosquitto/chain.pem", "/etc/ssl/certs", "/home/vaavva/repos/Mosquitto/vehicle01.pem", "/home/vaavva/repos/Mosquitto/vehicle01.key", NULL);
	if(rc != MOSQ_ERR_SUCCESS){
			mosquitto_destroy(mosq);
			printf("tls Error: %s\n", mosquitto_strerror(rc));
			return 1;
		}
#endif
	rc = mosquitto_connect(mosq, ADDRESS, PORT, 60);

	if(rc != MOSQ_ERR_SUCCESS){
			mosquitto_destroy(mosq);
			printf("conn Error: %s\n", mosquitto_strerror(rc));
			return 1;
		}

	mosquitto_loop_forever(mosq, -1, 1);

	mosquitto_lib_cleanup();
	return 0;
}