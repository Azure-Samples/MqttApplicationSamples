/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mosquitto.h"
#include "mqtt_callbacks.h"
#include "mqtt_protocol.h"
#include "mqtt_setup.h"

#include "unlock_command.pb-c.h"

#define QOS_LEVEL 1
#define MQTT_VERSION MQTT_PROTOCOL_V5

#define COMMAND_CONTENT_TYPE "application/protobuf"

#define RETURN_IF_ERROR(rc)                                           \
  do                                                                  \
  {                                                                   \
    if (rc != MOSQ_ERR_SUCCESS)                                       \
    {                                                                 \
      printf("[ERROR] Failure while sending response: %s\n", mosquitto_strerror(rc)); \
      free(response_topic);                                           \
      response_topic = NULL;                                          \
      free(correlation_data);                                         \
      correlation_data = NULL;                                        \
      mosquitto_property_free_all(&response_props);                   \
      response_props = NULL;                                          \
      return;                                                         \
    }                                                                 \
  } while (0)

// Function to execute unlock request. For this sample, it just prints the request information.
bool handle_unlock(char* payload, int payload_length)
{
  UnlockRequest* unlock_request = unlock_request__unpack(NULL, payload_length, payload);
  if (unlock_request == NULL)
  {
    printf("[ERROR] Failure unpacking protobuf payload\n");
    return false;
  }
  else
  {
    printf("\tUnlock request sent from %s at %s", unlock_request->requestedfrom, asctime( localtime(&unlock_request->when->seconds)));
    printf("[Server] Vehicle successfully unlocked\n");
    return true;
  }
}

// Custom callback for when a message is received.
// Prints the command request information and sends the response.
void handle_message(
    struct mosquitto* mosq,
    const struct mosquitto_message* message,
    const mosquitto_property* props)
{
  char* response_topic;
  void* correlation_data;
  uint16_t correlation_data_len;
  mosquitto_property* response_props = NULL;

  bool command_succeed = handle_unlock(message->payload, message->payloadlen);

  if (mosquitto_property_read_string(props, MQTT_PROP_RESPONSE_TOPIC, &response_topic, false)
      == NULL)
  {
    printf("[ERROR] Message does not have a response topic property\n");
    return;
  }

  printf("\tresponse_topic: %s\n", response_topic);

  if (mosquitto_property_read_binary(
          props, MQTT_PROP_CORRELATION_DATA, &correlation_data, &correlation_data_len, false)
      == NULL)
  {
    printf("[ERROR] Message does not have a correlation data property\n");
    return;
  }

  RETURN_IF_ERROR(mosquitto_property_add_binary(
      &response_props, MQTT_PROP_CORRELATION_DATA, correlation_data, correlation_data_len));
  RETURN_IF_ERROR(
      mosquitto_property_add_string(&response_props, MQTT_PROP_CONTENT_TYPE, COMMAND_CONTENT_TYPE));

  UnlockResponse unlock_response = UNLOCK_RESPONSE__INIT;
  void * buf;
  unsigned len;
  unlock_response.succeed = command_succeed;
  if (command_succeed == false)
  {
    unlock_response.errordetail = "Error executing unlock request";
  }
  len = unlock_response__get_packed_size(&unlock_response);
  buf = malloc(len);
  unlock_response__pack(&unlock_response, buf);
  printf("[Server] Sending unlock response:\n\tSucceed: %s\n", unlock_response.succeed ? "True" : "False");
  if (command_succeed == false)
  {
    printf("\tError: %s\n", unlock_response.errordetail);
  }

  RETURN_IF_ERROR(mosquitto_publish_v5(mosq, NULL, response_topic, len, buf, QOS_LEVEL, false, response_props));

  free(response_topic);
  response_topic = NULL;
  free(correlation_data);
  correlation_data = NULL;
  mosquitto_property_free_all(&response_props);
  response_props = NULL;
  free(buf);
  buf = NULL;
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
  mqtt_client_obj* client_obj = (mqtt_client_obj*)obj;
  char sub_topic[strlen(client_obj->client_id) + 33];
  sprintf(sub_topic, "vehicles/%s/command/unlock/request", client_obj->client_id);

  /* Making subscriptions in the on_connect() callback means that if the
   * connection drops and is automatically resumed by the client, then the
   * subscriptions will be recreated when the client reconnects. */
  if (keep_running
      && (result = mosquitto_subscribe_v5(mosq, NULL, sub_topic, QOS_LEVEL, 0, NULL))
          != MOSQ_ERR_SUCCESS)
  {
    printf("[ERROR] Failed to subscribe: %s\n", mosquitto_strerror(result));
    keep_running = 0;
    /* We might as well disconnect if we were unable to subscribe */
    if ((result = mosquitto_disconnect_v5(mosq, reason_code, props)) != MOSQ_ERR_SUCCESS)
    {
      printf("[ERROR] Failed to disconnect: %s\n", mosquitto_strerror(result));
    }
  }
}

/*
 * This sample receives commands from a client and responds.
 */
int main(int argc, char* argv[])
{
  struct mosquitto* mosq;
  int result = MOSQ_ERR_SUCCESS;

  mqtt_client_obj obj;
  obj.handle_message = handle_message;
  obj.mqtt_version = MQTT_VERSION;

  if ((mosq = mqtt_client_init(true, argv[1], on_connect_with_subscribe, &obj)) == NULL)
  {
    result = MOSQ_ERR_UNKNOWN;
  }
  else if (
      (result = mosquitto_connect_bind_v5(
           mosq, obj.hostname, obj.tcp_port, obj.keep_alive_in_seconds, NULL, NULL))
      != MOSQ_ERR_SUCCESS)
  {
    printf("[ERROR] Failed to connect: %s\n", mosquitto_strerror(result));
    result = MOSQ_ERR_UNKNOWN;
  }
  else if ((result = mosquitto_loop_start(mosq)) != MOSQ_ERR_SUCCESS)
  {
    printf("[ERROR] Failure in mosquitto loop: %s\n", mosquitto_strerror(result));
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
