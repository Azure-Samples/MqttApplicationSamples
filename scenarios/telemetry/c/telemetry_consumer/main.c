/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <stdlib.h>

#include "geo_json_handler.h"
#include "logging.h"
#include "mosquitto.h"
#include "mqtt_callbacks.h"
#include "mqtt_setup.h"

#define SUB_TOPIC "vehicles/+/position"
#define QOS_LEVEL 1
#define MQTT_VERSION MQTT_PROTOCOL_V311

// Custom callback for when a message is received.
void print_point_telemetry_message(
    struct mosquitto* mosq,
    const struct mosquitto_message* message,
    const mosquitto_property* props)
{
  geojson_point json_message = geojson_point_init();

  int rc = mosquitto_payload_to_geojson_point(message, &json_message);
  if (rc == 0)
  {
    printf("\ttype: %s\n", json_message.type);
    printf("\tcoordinates: %f, %f\n", json_message.coordinates.x, json_message.coordinates.y);
  }
  else
  {
    LOG_ERROR("Failure parsing JSON: %s", (char*)message->payload);
  }

  geojson_point_destroy(&json_message);
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
  if (keep_running
      && (result = mosquitto_subscribe_v5(mosq, NULL, SUB_TOPIC, QOS_LEVEL, 0, NULL))
          != MOSQ_ERR_SUCCESS)
  {
    LOG_ERROR("Failed to subscribe: %s", mosquitto_strerror(result));
    keep_running = 0;
    /* We might as well disconnect if we were unable to subscribe */
    if ((result = mosquitto_disconnect_v5(mosq, reason_code, props)) != MOSQ_ERR_SUCCESS)
    {
      LOG_ERROR("Failed to disconnect: %s", mosquitto_strerror(result));
    }
  }
}

/*
 * This sample receives telemetry messages from the broker.
 */
int main(int argc, char* argv[])
{
  struct mosquitto* mosq;
  int result = MOSQ_ERR_SUCCESS;

  mqtt_client_obj obj;
  obj.handle_message = print_point_telemetry_message;
  obj.mqtt_version = MQTT_VERSION;

  if ((mosq = mqtt_client_init(false, argv[1], on_connect_with_subscribe, &obj)) == NULL)
  {
    result = MOSQ_ERR_UNKNOWN;
  }
  else if (
      (result = mosquitto_connect_bind_v5(
           mosq, obj.hostname, obj.tcp_port, obj.keep_alive_in_seconds, NULL, NULL))
      != MOSQ_ERR_SUCCESS)
  {
    LOG_ERROR("Failed to connect: %s", mosquitto_strerror(result));
    result = MOSQ_ERR_UNKNOWN;
  }
  else if ((result = mosquitto_loop_start(mosq)) != MOSQ_ERR_SUCCESS)
  {
    LOG_ERROR("Failure starting mosquitto loop: %s", mosquitto_strerror(result));
    result = MOSQ_ERR_UNKNOWN;
  }
  else
  {
    while (keep_running)
    {
    }
  }

  if (mosq != NULL)
  {
    mosquitto_disconnect_v5(mosq, result, NULL);
    mosquitto_loop_stop(mosq, false);
    mosquitto_destroy(mosq);
  }
  mosquitto_lib_cleanup();
  return result;
}
