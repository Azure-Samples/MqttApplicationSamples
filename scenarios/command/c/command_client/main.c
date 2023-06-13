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
#include "unlock_command.pb-c.h"

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

static uuid_t current_correlation_id;

// Custom callback for when a message is received.
// prints the message information from the command response and validates that the correlation data
// matches.
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
  if (uuid_compare(current_correlation_id, correlation_data) != 0)
  {
    uuid_unparse(current_correlation_id, readable_correlation_data);
    printf("\t[ERROR] Correlation data does not match, expected: %s\n", readable_correlation_data);
  }

  free(correlation_data);
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
  //     sprintf(sub_topic, "vehicles/%s/command/unlock/response", client_obj->client_id);

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
    // sprintf(topic, "vehicles/%s/command/unlock/request", obj->client_id);

    // char response_topic[strlen(obj->client_id) + 33];
    // sprintf(response_topic, "vehicles/%s/command/unlock/response", obj->client_id);

    mosquitto_property* proplist = NULL;

    while (keep_running)
    {
      CONTINUE_IF_ERROR(
          mosquitto_property_add_string(&proplist, MQTT_PROP_RESPONSE_TOPIC, RESPONSE_TOPIC));
      CONTINUE_IF_ERROR(
          mosquitto_property_add_string(&proplist, MQTT_PROP_CONTENT_TYPE, COMMAND_CONTENT_TYPE));
      uuid_generate(current_correlation_id);

      CONTINUE_IF_ERROR(mosquitto_property_add_binary(
          &proplist, MQTT_PROP_CORRELATION_DATA, current_correlation_id, UUID_LENGTH));
      
      UnlockRequest unlock_request = UNLOCK_REQUEST__INIT;
      void * buf;
      unsigned len;
      unlock_request.requestedfrom = obj->client_id;
      Google__Protobuf__Timestamp timestamp = GOOGLE__PROTOBUF__TIMESTAMP__INIT;
      timestamp.seconds = time(NULL);
      timestamp.nanos = 0;
      unlock_request.when = &timestamp;
      len = unlock_request__get_packed_size(&unlock_request);
      buf = malloc(len);
      unlock_request__pack(&unlock_request, buf);
      printf("unlock payload: %s\n", (char*)buf);

      CONTINUE_IF_ERROR(mosquitto_publish_v5(
          mosq, NULL, PUB_TOPIC, len, buf, QOS, false, proplist));

      mosquitto_property_free_all(&proplist);

      sleep(2);
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
