// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <mosquitto.h>

#define USE_SSL

#ifdef USE_SSL
  #define ADDRESS     "localhost"
  #define PORT        8883
#else
  #define ADDRESS     "localhost"
  #define PORT        1883
#endif

#define CLIENTID    "ExampleClientPub"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

static int msgCount = 0;

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
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
}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
  printf("DISCONNECT reason=%d\n", rc);
}


/* Callback called when the client knows to the best of its abilities that a
 * PUBLISH has been successfully sent. For QoS 0 this means the message has
 * been completely written to the operating system. For QoS 1 this means we
 * have received a PUBACK from the broker. For QoS 2 this means we have
 * received a PUBCOMP from the broker. */
void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
  msgCount++;
	printf("Message with mid %d has been published.\n", mid);
}


/*
 * This sample sends five telemetry messages to the Broker. X509 self-certification is used.
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
	mosquitto_publish_callback_set(mosq, on_publish);

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

  rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		printf("loop Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}
  char *topic = TOPIC;
  char *payload = PAYLOAD;

  while(msgCount < 5)
  {
	sleep(1);
    printf("Publishing message\n");
		rc = mosquitto_publish(mosq, NULL, TOPIC, (int)strlen(PAYLOAD), PAYLOAD, QOS, false);
    if(rc != MOSQ_ERR_SUCCESS){
      printf("Error publishing: %s\n", mosquitto_strerror(rc));
    }
	}

	mosquitto_lib_cleanup();
	return 0;
}
