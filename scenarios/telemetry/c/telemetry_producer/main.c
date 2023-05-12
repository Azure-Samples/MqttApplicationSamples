/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mosquitto.h"
#include "mqtt_setup.h"

#define PAYLOAD "{\"type\":\"Point\",\"coordinates\":[-2.124156,51.899523]}"
#define QOS 1
#define MQTT_VERSION MQTT_PROTOCOL_V311

/*
 * This sample sends telemetry messages to the Broker. X509 authentication is used.
 */
int main(int argc, char* argv[])
{
  struct mosquitto* mosq;
  int result = MOSQ_ERR_SUCCESS;

  mqtt_client_obj* obj = calloc(1, sizeof(mqtt_client_obj));
  obj->print_message = NULL;
  obj->mqtt_version = MQTT_VERSION;

  if ((mosq = mqtt_client_init(true, argv[1], NULL, obj)) == NULL)
  {
    result = MOSQ_ERR_UNKNOWN;
  }
  else if (
      (result = mosquitto_connect_bind_v5(
           mosq, obj->hostname, obj->tcp_port, obj->keep_alive_in_seconds, NULL, NULL))
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
    char topic[strlen(connection_settings->client_id) + 17];
    sprintf(topic, "vehicles/%s/position", connection_settings->client_id);

    while (keep_running)
    {
      result = mosquitto_publish_v5(
          mosq, NULL, topic, (int)strlen(PAYLOAD), PAYLOAD, QOS, false, NULL);

      if (result != MOSQ_ERR_SUCCESS)
      {
        printf("Error publishing: %s\n", mosquitto_strerror(result));
      }

      sleep(5);
    }
  }

  if (mosq != NULL)
  {
    mosquitto_disconnect_v5(mosq, result, NULL);
    mosquitto_loop_stop(mosq, false);
    mosquitto_destroy(mosq);
  }
  mosquitto_lib_cleanup();
  free(obj);
  return result;
}
