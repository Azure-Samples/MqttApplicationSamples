/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mosquitto.h"
#include "mqtt_callbacks.h"
#include "mqtt_setup.h"

#define PUB_TOPIC "sample/1"
#define PAYLOAD "Hello World!"
#define SUB_TOPIC "sample/+"
#define QOS 1
#define MQTT_VERSION MQTT_PROTOCOL_V311

static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
  (void)_;
  keep_running = 0;
}

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
  if ((result = mosquitto_subscribe_v5(mosq, NULL, SUB_TOPIC, QOS, 0, NULL)) != MOSQ_ERR_SUCCESS)
  {
    printf("Error subscribing: %s\n", mosquitto_strerror(result));
    /* We might as well disconnect if we were unable to subscribe */
    if ((result = mosquitto_disconnect_v5(mosq, reason_code, props)) != MOSQ_ERR_SUCCESS)
    {
      printf("Error disconnecting: %s\n", mosquitto_strerror(result));
    }
  }
}

void cleanup(
    struct mosquitto* mosq,
    mqtt_client_obj* obj,
    mqtt_client_connection_settings* connection_settings)
{
  if (mosq != NULL)
  {
    mosquitto_disconnect_v5(mosq, MOSQ_ERR_SUCCESS, NULL);
    mosquitto_loop_stop(mosq, true);
    mosquitto_destroy(mosq);
  }
  mosquitto_lib_cleanup();
  free(connection_settings);
  free(obj);
}

/*
 * This sample sends and receives messages to/from the Broker. X509 authentication is used.
 * @return MOSQ_ERR_SUCCESS (0) on success, other enum mosq_err_t on failure
 */
int main(int argc, char* argv[])
{
  signal(SIGINT, sig_handler);

  struct mosquitto* mosq;
  int result = MOSQ_ERR_SUCCESS;
  mqtt_client_connection_settings* connection_settings
      = calloc(1, sizeof(mqtt_client_connection_settings));

  mqtt_client_obj* obj = calloc(1, sizeof(mqtt_client_obj));
  obj->print_message = NULL;
  obj->mqtt_version = MQTT_VERSION;

  if ((mosq = mqtt_client_init(true, argv[1], on_connect_with_subscribe, obj, connection_settings))
      == NULL)
  {
    result = MOSQ_ERR_UNKNOWN;
  }
  else if (
      (result = mosquitto_connect_bind_v5(
           mosq,
           connection_settings->hostname,
           connection_settings->tcp_port,
           connection_settings->keep_alive_in_seconds,
           NULL,
           NULL))
      != MOSQ_ERR_SUCCESS)
  {
    printf("Connection Error: %s\n", mosquitto_strerror(result));
    result = MOSQ_ERR_UNKNOWN;
  }
  else if ((result = mosquitto_loop_start(mosq)) != MOSQ_ERR_SUCCESS)
  {
    printf("loop Error: %s\n", mosquitto_strerror(result));
    result = MOSQ_ERR_UNKNOWN;
  }
  else
  {
    while (keep_running)
    {
      result = mosquitto_publish_v5(
          mosq, NULL, PUB_TOPIC, (int)strlen(PAYLOAD), PAYLOAD, QOS, false, NULL);

      if (result != MOSQ_ERR_SUCCESS)
      {
        printf("Error publishing: %s\n", mosquitto_strerror(result));
      }

      sleep(5);
    }
  }

  cleanup(mosq, obj, connection_settings);
  return result;
}
