/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "mosquitto.h"
#include "mqtt_callbacks.h"
#include "mqtt_protocol.h"
#include "mqtt_setup.h"

#define PAYLOAD "\n\v\b\261Í\244\006\020\364\254\265d\022\nmobile-app"
#define RESPONSE_TOPIC "vehicles/vehicle03/command/unlock/response"
#define PUB_TOPIC "vehicles/vehicle03/command/unlock/request"
#define COMMAND_CONTENT_TYPE "application/protobuf"

#define QOS 1
#define MQTT_VERSION MQTT_PROTOCOL_V5
#define UUID_LENGTH 16

#define CONTINUE_IF_ERROR(rc)                                   \
  do                                                            \
  {                                                             \
    if (rc != MOSQ_ERR_SUCCESS)                                 \
    {                                                           \
      printf("Error publishing: %s\n", mosquitto_strerror(rc)); \
      mosquitto_property_free_all(&proplist);                   \
      continue;                                                 \
    }                                                           \
  } while (0)

// Custom callback for when a message is received.
void handle_message(
    struct mosquitto* mosq,
    const struct mosquitto_message* message,
    const mosquitto_property* props)
{
  printf(
      "on_message: Topic: %s; QOS: %d; protobuf Payload: %s\n",
      message->topic,
      message->qos,
      (char*)message->payload);

  void* correlation_data;
  uint16_t correlation_data_len;
  if (mosquitto_property_read_binary(
          props, MQTT_PROP_CORRELATION_DATA, &correlation_data, &correlation_data_len, false)
      == NULL)
  {
    printf("Error reading correlation data\n");
    return;
  }

  char readable_correlation_data[correlation_data_len];
  uuid_unparse(correlation_data, readable_correlation_data);
  printf("\tcorr_data: %s\n", readable_correlation_data);
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
  //   mqtt_client_obj* client_obj = (mqtt_client_obj*)obj;
  //   char sub_topic[strlen(client_obj->client_id) + 33];
  //     sprintf(sub_topic, "vehicles/vehicle01/commands/unlock/response", client_obj->client_id);

  /* Making subscriptions in the on_connect() callback means that if the
   * connection drops and is automatically resumed by the client, then the
   * subscriptions will be recreated when the client reconnects. */
  if (keep_running
      && (result = mosquitto_subscribe_v5(mosq, NULL, RESPONSE_TOPIC, QOS, 0, NULL))
          != MOSQ_ERR_SUCCESS)
  {
    printf("Error subscribing: %s\n", mosquitto_strerror(result));
    keep_running = 0;
    /* We might as well disconnect if we were unable to subscribe */
    if ((result = mosquitto_disconnect_v5(mosq, reason_code, props)) != MOSQ_ERR_SUCCESS)
    {
      printf("Error disconnecting: %s\n", mosquitto_strerror(result));
    }
  }
}

/*
 * This sample sends an unlock command to the vehicle.
 */
int main(int argc, char* argv[])
{
  struct mosquitto* mosq;
  int result = MOSQ_ERR_SUCCESS;

  mqtt_client_obj* obj = calloc(1, sizeof(mqtt_client_obj));
  obj->mqtt_version = MQTT_VERSION;
  obj->handle_message = handle_message;

  if ((mosq = mqtt_client_init(true, argv[1], on_connect_with_subscribe, obj)) == NULL)
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
    // char topic[strlen(obj->client_id) + 33];
    // sprintf(topic, "vehicles/vehicle03/commands/unlock/request", obj->client_id);

    // char response_topic[strlen(obj->client_id) + 33];
    // sprintf(response_topic, "vehicles/vehicle03/commands/unlock/response", obj->client_id);

    mosquitto_property* proplist = NULL;
    uuid_t corr_data;

    while (keep_running)
    {
      CONTINUE_IF_ERROR(
          mosquitto_property_add_string(&proplist, MQTT_PROP_RESPONSE_TOPIC, RESPONSE_TOPIC));
      CONTINUE_IF_ERROR(
          mosquitto_property_add_string(&proplist, MQTT_PROP_CONTENT_TYPE, COMMAND_CONTENT_TYPE));
      uuid_generate(corr_data);
      CONTINUE_IF_ERROR(mosquitto_property_add_binary(
          &proplist, MQTT_PROP_CORRELATION_DATA, corr_data, UUID_LENGTH));

      CONTINUE_IF_ERROR(mosquitto_publish_v5(
          mosq, NULL, PUB_TOPIC, (int)strlen(PAYLOAD), PAYLOAD, QOS, false, proplist));

      mosquitto_property_free_all(&proplist);

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
