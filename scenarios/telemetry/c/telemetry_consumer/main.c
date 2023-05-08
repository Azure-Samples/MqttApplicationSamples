/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <stdlib.h>

#include "mqtt_callbacks.h"
#include "mqtt_setup.h"
#include <mosquitto.h>

#define SUB_TOPIC "vehicles/+/position"
#define QOS 1
#define MQTT_VERSION MQTT_PROTOCOL_V311

// Custom callback for when a message is received.
void print_message(const struct mosquitto_message* message)
{
  printf(
      "on_message: Topic: %s; QOS: %d; JSON Payload: %s\n",
      message->topic,
      message->qos,
      (char*)message->payload);
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
  result = mosquitto_subscribe_v5(mosq, NULL, SUB_TOPIC, QOS, 0, NULL);

  if (result != MOSQ_ERR_SUCCESS)
  {
    printf("Error subscribing: %s\n", mosquitto_strerror(result));
    /* We might as well disconnect if we were unable to subscribe */
    mosquitto_disconnect_v5(mosq, reason_code, props);
  }
}

/*
 * This sample receives telemetry messages from the broker. X509 authentication is used.
 */
int main(int argc, char* argv[])
{
  struct mosquitto* mosq;
  int result = 0;
  mqtt_client_connection_settings* connection_settings
      = calloc(1, sizeof(mqtt_client_connection_settings));

  mqtt_client_obj* obj = calloc(1, sizeof(mqtt_client_obj));
  obj->print_message = print_message;
  obj->mqtt_version = MQTT_VERSION;

  mosq = mqtt_client_init(false, argv[1], on_connect_with_subscribe, obj, connection_settings);
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

  mosquitto_loop_forever(mosq, -1, 1);

  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();
  free(connection_settings);
  return 0;
}
