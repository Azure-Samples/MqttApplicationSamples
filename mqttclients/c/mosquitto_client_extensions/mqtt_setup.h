/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#ifndef MQTT_SETUP_H
#define MQTT_SETUP_H

#include "mosquitto.h"
#include <signal.h>
#include <stdbool.h>

extern volatile sig_atomic_t keep_running;

typedef struct mqtt_client_connection_settings
{
  char* ca_file;
  char* cert_file;
  char* client_id;
  char* hostname;
  char* key_file;
  char* key_file_password;
  char* password;
  char* username;
  int keep_alive_in_seconds;
  int tcp_port;
  bool clean_session;
  bool use_TLS;
} mqtt_client_connection_settings;

typedef struct mqtt_client_obj
{
  void (*print_message)(const struct mosquitto_message* message);
  char* client_id;
  char* hostname;
  int keep_alive_in_seconds;
  int mqtt_version;
  int tcp_port;
} mqtt_client_obj;

struct mosquitto* mqtt_client_init(
    bool publish,
    char* env_file,
    void (*on_connect_with_subscribe)(
        struct mosquitto*,
        void*,
        int,
        int,
        const mosquitto_property* props),
    mqtt_client_obj* mqtt_client_obj);

#endif /* MQTT_SETUP_H */
