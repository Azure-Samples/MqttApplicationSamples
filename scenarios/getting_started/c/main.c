/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mqtt_callbacks.h"
#include "mqtt_setup.h"
#include <mosquitto.h>

#define PUB_TOPIC "sample/1"
#define PAYLOAD "Hello World!"
#define SUB_TOPIC "sample/+"
#define QOS 1

/* Callback called when the client receives a CONNACK message from the broker and we want to
 * subscribe on connect. */
void on_connect_with_subscribe(
    struct mosquitto* mosq,
    void* obj,
    int reason_code,
    int flags,
    const mosquitto_property* props)
{
  on_connect(mosq, obj, reason_code, flags, props);

  int result;

  /* Making subscriptions in the on_connect() callback means that if the
   * connection drops and is automatically resumed by the client, then the
   * subscriptions will be recreated when the client reconnects. */
  result = mosquitto_subscribe_v5(mosq, NULL, SUB_TOPIC, QOS, 0, NULL);

  if (result != MOSQ_ERR_SUCCESS)
  {
    printf("Error subscribing: %s\n", mosquitto_strerror(result));
    /* We might as well disconnect if we were unable to subscribe */
    mosquitto_disconnect_v5(mosq, reason_code, props);
  }
}

/*
 * This sample sends and receives messages to/from the Broker. X509 authentication is used.
 */
int main(int argc, char* argv[])
{
  struct mosquitto* mosq;
  int result = 0;
  mqtt_client_connection_settings* connection_settings
      = calloc(1, sizeof(mqtt_client_connection_settings));

  mosq = mqtt_client_init(true, argv[1], on_connect_with_subscribe, NULL, connection_settings);
  result = mosquitto_connect_bind_v5(
      mosq,
      connection_settings->hostname,
      connection_settings->tcp_port,
      connection_settings->keep_alive_in_seconds,
      NULL,
      NULL);

  if (result != MOSQ_ERR_SUCCESS)
  {
    mosquitto_destroy(mosq);
    printf("Connection Error: %s\n", mosquitto_strerror(result));
    return 1;
  }

  result = mosquitto_loop_start(mosq);

  if (result != MOSQ_ERR_SUCCESS)
  {
    mosquitto_destroy(mosq);
    printf("loop Error: %s\n", mosquitto_strerror(result));
    return 1;
  }

  while (true)
  {
    result = mosquitto_publish_v5(
        mosq, NULL, PUB_TOPIC, (int)strlen(PAYLOAD), PAYLOAD, QOS, false, NULL);

    if (result != MOSQ_ERR_SUCCESS)
    {
      printf("Error publishing: %s\n", mosquitto_strerror(result));
    }

    sleep(5);
  }

  mosquitto_loop_stop(mosq, true);

  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();
  free(connection_settings);
  return 0;
}
