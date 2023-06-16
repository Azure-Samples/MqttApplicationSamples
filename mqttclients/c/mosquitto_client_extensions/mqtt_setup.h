/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#ifndef MQTT_SETUP_H
#define MQTT_SETUP_H

#include "mosquitto.h"
#include <signal.h>
#include <stdbool.h>

#define DEFAULT_TCP_PORT 8883
#define DEFAULT_KEEP_ALIVE_IN_SECONDS 30
#define DEFAULT_USE_TLS true
#define DEFAULT_CLEAN_SESSION true

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
  void (*handle_message)(
      struct mosquitto*,
      const struct mosquitto_message*,
      const mosquitto_property*);
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

bool set_char_connection_setting(
    char** connection_setting,
    const char* env_name,
    bool fail_not_defined);

bool set_int_connection_setting(int* connection_setting, char* env_name, int default_value);

bool set_bool_connection_setting(bool* connection_setting, char* env_name, bool default_value);

bool mqtt_client_set_connection_settings(mqtt_client_connection_settings* connection_settings);

#endif /* MQTT_SETUP_H */
