/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#ifndef MQTT_CALLBACKS_H
#define MQTT_CALLBACKS_H

#include "mosquitto.h"

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(
    struct mosquitto* mosq,
    void* obj,
    int reason_code,
    int flags,
    const mosquitto_property* props);

/* Callback called when the broker has received the DISCONNECT command and has disconnected the
 * client. */
void on_disconnect(struct mosquitto* mosq, void* obj, int rc, const mosquitto_property* props);

/* Callback called when the broker sends a SUBACK in response to a SUBSCRIBE. */
void on_subscribe(
    struct mosquitto* mosq,
    void* obj,
    int mid,
    int qos_count,
    const int* granted_qos,
    const mosquitto_property* props);

/* Callback called when the client receives a message. */
void on_message(
    struct mosquitto* mosq,
    void* obj,
    const struct mosquitto_message* msg,
    const mosquitto_property* props);

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
    const mosquitto_property* props);

/* Callback to enter the key file password. */
int key_file_password_callback(char* buf, int size, int rwflag, void* userdata);

#endif /* MQTT_CALLBACKS_H */
