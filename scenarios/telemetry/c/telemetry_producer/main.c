/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "geo_json_handler.h"
#include "mosquitto.h"
#include "mqtt_setup.h"

#define QOS_LEVEL 1
#define MQTT_VERSION MQTT_PROTOCOL_V311

double generate_random_coordinate()
{
  double scale = rand() / (double)RAND_MAX;
  return (scale * (180)) - 90;
}

/*
 * This sample sends telemetry messages to the Broker.
 */
int main(int argc, char* argv[])
{
  struct mosquitto* mosq;
  int result = MOSQ_ERR_SUCCESS;

  mqtt_client_obj obj;
  obj.mqtt_version = MQTT_VERSION;

  if ((mosq = mqtt_client_init(true, argv[1], NULL, &obj)) == NULL)
  {
    result = MOSQ_ERR_UNKNOWN;
  }
  else if (
      (result = mosquitto_connect_bind_v5(
           mosq, obj.hostname, obj.tcp_port, obj.keep_alive_in_seconds, NULL, NULL))
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
    char topic[strlen(obj.client_id) + 17];
    sprintf(topic, "vehicles/%s/position", obj.client_id);
    mosquitto_payload payload = mosquitto_payload_init(60);
    geojson_point geojson_point = geojson_point_init();

    while (keep_running)
    {
      geojson_point_set_coordinates(
          &geojson_point, generate_random_coordinate(), generate_random_coordinate());
      if (geojson_point_to_mosquitto_payload(geojson_point, &payload) != 0)
      {
        result = MOSQ_ERR_UNKNOWN;
      }
      else
      {
        result = mosquitto_publish_v5(
            mosq, NULL, topic, payload.payload_length, payload.payload, QOS_LEVEL, false, NULL);
      }

      if (result != MOSQ_ERR_SUCCESS)
      {
        printf("Error publishing: %s\n", mosquitto_strerror(result));
      }

      sleep(5);
    }
    mosquitto_payload_destroy(&payload);
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
