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

#define COMMAND_TARGET_CLIENT_ID "vehicle03"
#define COMMAND_TARGET_CLIENT_ID_LEN 9
#define COMMAND_CONTENT_TYPE "application/protobuf"
#define COMMAND_TIMEOUT_SEC 10
#define COMMAND_MIN_RATE_SEC 2

#define QOS_LEVEL 1
#define MQTT_VERSION MQTT_PROTOCOL_V5

#define UUID_LENGTH 37

#define CONTINUE_IF_ERROR(rc)                                                   \
  if (true)                                                                     \
  {                                                                             \
    if (rc != MOSQ_ERR_SUCCESS)                                                 \
    {                                                                           \
      printf("[ERROR] Failure while publishing: %s\n", mosquitto_strerror(rc)); \
      mosquitto_property_free_all(&proplist);                                   \
      proplist = NULL;                                                          \
      free(payload_buf);                                                        \
      payload_buf = NULL;                                                       \
      continue;                                                                 \
    }                                                                           \
  }

static uuid_t pending_correlation_id;
static time_t last_command_sent_time;
static char response_topic[COMMAND_TARGET_CLIENT_ID_LEN + 34];

char* get_response_topic()
{
  if (strlen(response_topic) == 0)
  {
    sprintf(response_topic, "vehicles/%s/command/unlock/response", COMMAND_TARGET_CLIENT_ID);
  }
  return response_topic;
}

// Custom callback for when a message is received.
// prints the message information from the command response and validates that the correlation data
// matches.
void handle_message(
    struct mosquitto* mosq,
    const struct mosquitto_message* message,
    const mosquitto_property* props)
{
  void* correlation_data;
  uint16_t correlation_data_len;

  // deserialize the protobuf payload
  UnlockResponse* unlock_response
      = unlock_response__unpack(NULL, message->payloadlen, message->payload);
  if (unlock_response == NULL)
  {
    printf("\t[ERROR] Failure deserializing protobuf payload\n");
  }
  else if (unlock_response->succeed == true)
  {
    printf("\tCommand succeed: True\n");
  }
  else
  {
    printf("\tCommand succeed: False\n\tError: %s\n", unlock_response->errordetail);
  }

  if (mosquitto_property_read_binary(
          props, MQTT_PROP_CORRELATION_DATA, &correlation_data, &correlation_data_len, false)
      == NULL)
  {
    printf("\t[ERROR] Message does not have a correlation data property\n");
    unlock_response__free_unpacked(unlock_response, NULL);
    unlock_response = NULL;
    return;
  }

  if (uuid_compare(pending_correlation_id, correlation_data) != 0)
  {
    char readable_correlation_data[UUID_LENGTH];
    uuid_unparse(pending_correlation_id, readable_correlation_data);
    printf("\t[ERROR] Correlation data does not match, expected: %s ", readable_correlation_data);
    uuid_unparse(correlation_data, readable_correlation_data);
    printf("received: %s\n", readable_correlation_data);
  }
  else
  {
    uuid_clear(pending_correlation_id);
  }

  free(correlation_data);
  correlation_data = NULL;
  unlock_response__free_unpacked(unlock_response, NULL);
  unlock_response = NULL;
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
      && (result = mosquitto_subscribe_v5(mosq, NULL, get_response_topic(), QOS_LEVEL, 0, NULL))
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
 * This sample sends an unlock command to the vehicle.
 */
int main(int argc, char* argv[])
{
  struct mosquitto* mosq;
  int result;

  mqtt_client_obj obj;
  obj.mqtt_version = MQTT_VERSION;
  obj.handle_message = handle_message;

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
    printf("[ERROR] Failure starting mosquitto loop: %s\n", mosquitto_strerror(result));
    result = MOSQ_ERR_UNKNOWN;
  }
  else
  {
    char pub_topic[COMMAND_TARGET_CLIENT_ID_LEN + 33];
    sprintf(pub_topic, "vehicles/%s/command/unlock/request", COMMAND_TARGET_CLIENT_ID);

    // Set up protobuf unlock payload
    UnlockRequest proto_unlock_request = UNLOCK_REQUEST__INIT;
    void* payload_buf;
    size_t proto_payload_len;
    Google__Protobuf__Timestamp proto_timestamp = GOOGLE__PROTOBUF__TIMESTAMP__INIT;
    proto_unlock_request.requestedfrom = obj.client_id;
    proto_timestamp.nanos = 0;

    mosquitto_property* proplist = NULL;
    time_t current_time;
    last_command_sent_time = time(0);
    uuid_clear(pending_correlation_id);

    while (keep_running)
    {
      current_time = time(NULL);
      // if there's a pending command
      if (!uuid_is_null(pending_correlation_id))
      {
        // wait until the command times out
        if (current_time < last_command_sent_time + COMMAND_TIMEOUT_SEC)
        {
          continue;
        }
        else
        {
          printf("[ERROR] Command timed out without a response.\n");
          uuid_clear(pending_correlation_id);
        }
      }
      // If the command timed out (didn't `continue` in the last if statement) or there is no
      // pending command, send a new command if it's been more than 2 seconds since the last command
      // (to avoid spamming commands)
      if (current_time > last_command_sent_time + COMMAND_MIN_RATE_SEC)
      {
        last_command_sent_time = current_time;

        proto_timestamp.seconds = current_time;
        proto_unlock_request.when = &proto_timestamp;
        proto_payload_len = unlock_request__get_packed_size(&proto_unlock_request);
        payload_buf = malloc(proto_payload_len);

        if (payload_buf == NULL)
        {
          printf("[ERROR] Failed to allocate memory for payload buffer.\n");
          continue;
        }

        if (unlock_request__pack(&proto_unlock_request, payload_buf) != proto_payload_len)
        {
          printf("[ERROR] Failure serializing payload.\n");
          free(payload_buf);
          payload_buf = NULL;
          continue;
        }

        CONTINUE_IF_ERROR(mosquitto_property_add_string(
            &proplist, MQTT_PROP_RESPONSE_TOPIC, get_response_topic()));
        CONTINUE_IF_ERROR(
            mosquitto_property_add_string(&proplist, MQTT_PROP_CONTENT_TYPE, COMMAND_CONTENT_TYPE));

        uuid_generate(pending_correlation_id);

        CONTINUE_IF_ERROR(mosquitto_property_add_binary(
            &proplist, MQTT_PROP_CORRELATION_DATA, pending_correlation_id, UUID_LENGTH));

        printf(
            "[Client] Sending unlock request from %s at %s",
            proto_unlock_request.requestedfrom,
            asctime(localtime(&proto_unlock_request.when->seconds)));

        CONTINUE_IF_ERROR(mosquitto_publish_v5(
            mosq, NULL, pub_topic, proto_payload_len, payload_buf, QOS_LEVEL, false, proplist));

        mosquitto_property_free_all(&proplist);
        proplist = NULL;

        free(payload_buf);
        payload_buf = NULL;
      }
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
