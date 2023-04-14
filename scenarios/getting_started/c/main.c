// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <mosquitto.h>
#include "setup.h"

#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"


/*
 * This sample sends and receives five telemetry messages to/from the Broker. X509 self-certification is used.
 */
int main(void)
{
  int port = atoi(getenv("BROKER_PORT"));
  int qos = atoi(getenv("QOS"));
  struct mosquitto *mosq;
	int rc=0;
  struct mosq_context *context = calloc(1, sizeof(struct mosq_context));
  context->messagesSent = 0;
  context->messagesReceived = 0;
  
  mosq = initMQTT(true, true, true, context);
  rc = mosquitto_connect(mosq, getenv("BROKER_ADDRESS"), port, 60);
  if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		printf("Connection Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

  rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		printf("loop Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

  while(context->messagesSent < 5)
  {
	  sleep(1);
    rc = mosquitto_publish(mosq, NULL, TOPIC, (int)strlen(PAYLOAD), PAYLOAD, qos, false);
    if(rc != MOSQ_ERR_SUCCESS){
      printf("Error publishing: %s\n", mosquitto_strerror(rc));
    }
	}
  mosquitto_loop_stop(mosq, false);

	mosquitto_lib_cleanup();
  free(context);
	return 0;
}
