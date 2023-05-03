/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mqtt_setup.h"
#include <mosquitto.h>

#define PAYLOAD "{\"type\":\"Point\",\"coordinates\":[-2.124156,51.899523]}"
#define QOS 1

/*
 * This sample sends telemetry messages to the Broker. X509 authentication is used.
 */
int main(int argc, char* argv[])
{
  struct mosquitto* mosq;
  int result = 0;
  mqtt_client_connection_settings* connection_settings
      = calloc(1, sizeof(mqtt_client_connection_settings));

  mosq = mqtt_client_init(true, argv[1], NULL, connection_settings);

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

  char topic[strlen(connection_settings->client_id) + 17];
  sprintf(topic, "vehicles/%s/position", connection_settings->client_id);

  while (true)
  {
    result
        = mosquitto_publish_v5(mosq, NULL, topic, (int)strlen(PAYLOAD), PAYLOAD, QOS, false, NULL);

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
