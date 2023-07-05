/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "mosquitto.h"
#include "mqtt_callbacks.h"
#include "mqtt_setup.h"

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(
    struct mosquitto* mosq,
    void* obj,
    int reason_code,
    int flags,
    const mosquitto_property* props)
{
  mqtt_client_obj* client_obj = (mqtt_client_obj*)obj;

  /* Print out the connection result. mosquitto_connack_string() produces an
   * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
   * clients is mosquitto_reason_string().
   */
  if (client_obj->mqtt_version == MQTT_PROTOCOL_V5)
  {
    LOG_INFO(MQTT_LOG_TAG, "on_connect: %s", mosquitto_reason_string(reason_code));
  }
  else
  {
    LOG_INFO(MQTT_LOG_TAG, "on_connect: %s", mosquitto_connack_string(reason_code));
  }

  if (reason_code != 0)
  {
    keep_running = 0;
    /* If the connection fails for any reason, we don't want to keep on
     * retrying in this example, so disconnect. Without this, the client
     * will attempt to reconnect. */
    int rc;
    if ((rc = mosquitto_disconnect_v5(mosq, reason_code, NULL)) != MOSQ_ERR_SUCCESS)
    {
      LOG_ERROR("Failure on disconnect: %s", mosquitto_strerror(rc));
    }
  }
}

/* Callback called when the broker has received the DISCONNECT command and has disconnected the
 * client. */
void on_disconnect(struct mosquitto* mosq, void* obj, int rc, const mosquitto_property* props)
{
  LOG_INFO(MQTT_LOG_TAG, "on_disconnect: reason=%s", mosquitto_strerror(rc));
}

/* Callback called when the broker sends a SUBACK in response to a SUBSCRIBE. */
void on_subscribe(
    struct mosquitto* mosq,
    void* obj,
    int mid,
    int qos_count,
    const int* granted_qos,
    const mosquitto_property* props)
{
  LOG_INFO(MQTT_LOG_TAG, "on_subscribe: Subscribed with mid %d; %d topics.", mid, qos_count);

  /* In this example we only subscribe to a single topic at once, but a
   * SUBSCRIBE can contain many topics at once, so this is one way to check
   * them all. */
  for (int i = 0; i < qos_count; i++)
  {
    printf("\tQoS %d\n", granted_qos[i]);
  }
}

/* Callback called when the client receives a message. */
void on_message(
    struct mosquitto* mosq,
    void* obj,
    const struct mosquitto_message* msg,
    const mosquitto_property* props)
{
  LOG_INFO(MQTT_LOG_TAG, "on_message: Topic: %s; QOS: %d; mid: %d", msg->topic, msg->qos, msg->mid);

  mqtt_client_obj* client_obj = (mqtt_client_obj*)obj;

  if (client_obj != NULL && client_obj->handle_message != NULL)
  {
    client_obj->handle_message(mosq, msg, props);
  }
  else
  {
    /* This blindly prints the payload, but the payload can be anything so take care. */
    printf("\tPayload: %s\n", (char*)msg->payload);
  }
}

/* Callback called when the client knows to the best of its abilities that a
 * PUBLISH has been successfully sent. For QoS 0 this means the message has
 * been completely written to the operating system. For QoS 1 this means we
 * have received a PUBACK from the broker. For QoS 2 this means we have
 * received a PUBCOMP from the broker. */
void on_publish(
    struct mosquitto* mosq,
    void* obj,
    int mid,
    int reason_code,
    const mosquitto_property* props)
{
  LOG_INFO(MQTT_LOG_TAG, "on_publish: Message with mid %d has been published.", mid);
}
