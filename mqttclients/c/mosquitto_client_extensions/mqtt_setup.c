/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mosquitto.h"
#include "mqtt_callbacks.h"
#include "mqtt_setup.h"

volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
  (void)_;
  keep_running = 0;
}

#define RETURN_IF_FAILED(rc)                     \
  do                                             \
  {                                              \
    enum mosq_err_t const mosq_result = (rc);    \
    if (mosq_result != MOSQ_ERR_SUCCESS)         \
    {                                            \
      if (mosq != NULL)                          \
      {                                          \
        mosquitto_destroy(mosq);                 \
      }                                          \
      printf(                                    \
          "Mosquitto Error: %s At [%s:%s:%d]\n", \
          mosquitto_strerror(mosq_result),       \
          __FILE__,                              \
          __func__,                              \
          __LINE__);                             \
      return NULL;                               \
    }                                            \
  } while (0)

void mqtt_client_read_env_file(char* file_path)
{
  /* If there was no env file_path passed in, look for a .env file in the current directory. */
  if (file_path == NULL)
  {
    file_path = ".env";
  }

  printf("Loading environment variables from %s\n", file_path);

  FILE* file_ptr = fopen(file_path, "r");

  if (file_ptr != NULL)
  {
    char env_string[300];
    char env_name[30];
    char env_value[256];

    while (fscanf(file_ptr, "%s", env_string) == 1)
    {
      char* env_name = strtok(env_string, "=");
      char* env_value = strtok(NULL, "=\"");
      setenv(env_name, env_value, 1);
    }

    fclose(file_ptr);
  }
  else
  {
    printf("Cannot open env file. Sample will try to use environment variables. \n");
  }
}

void mqtt_client_set_connection_settings(mqtt_client_connection_settings* connection_settings)
{
  connection_settings->hostname = getenv("MQTT_HOST_NAME");
  printf("MQTT_HOST_NAME = %s\n", connection_settings->hostname);

  connection_settings->tcp_port = atoi(getenv("MQTT_TCP_PORT") ?: "8883");
  printf("MQTT_TCP_PORT = %d\n", connection_settings->tcp_port);

  connection_settings->client_id = getenv("MQTT_CLIENT_ID");
  printf("MQTT_CLIENT_ID = %s\n", connection_settings->client_id);

  connection_settings->ca_file = getenv("MQTT_CA_FILE");
  printf("MQTT_CA_FILE = %s\n", connection_settings->ca_file);

  connection_settings->ca_path = getenv("MQTT_CA_PATH");
  printf("MQTT_CA_PATH = %s\n", connection_settings->ca_path);

  connection_settings->cert_file = getenv("MQTT_CERT_FILE");
  printf("MQTT_CERT_FILE = %s\n", connection_settings->cert_file);

  connection_settings->key_file = getenv("MQTT_KEY_FILE");
  printf("MQTT_KEY_FILE = %s\n", connection_settings->key_file);

  connection_settings->key_file_password = getenv("MQTT_KEY_FILE_PASSWORD");
  printf("MQTT_KEY_FILE_PASSWORD = %s\n", connection_settings->key_file_password);

  connection_settings->keep_alive_in_seconds = atoi(getenv("MQTT_KEEP_ALIVE_IN_SECONDS") ?: "30");
  printf("MQTT_KEEP_ALIVE_IN_SECONDS = %d\n", connection_settings->keep_alive_in_seconds);

  char* use_TLS = getenv("MQTT_USE_TLS");
  connection_settings->use_TLS = (use_TLS != NULL && strcmp(use_TLS, "false") == 0) ? false : true;
  printf("MQTT_USE_TLS = %s\n", connection_settings->use_TLS ? "true" : "false");

  connection_settings->username = getenv("MQTT_USERNAME");
  printf("MQTT_USERNAME = %s\n", connection_settings->username);

  connection_settings->password = getenv("MQTT_PASSWORD");
  printf("MQTT_PASSWORD = %s\n", connection_settings->password);

  char* clean_session = getenv("MQTT_CLEAN_SESSION");
  connection_settings->clean_session
      = (clean_session != NULL && strcmp(clean_session, "false") == 0) ? false : true;
  printf("MQTT_CLEAN_SESSION = %s\n", connection_settings->clean_session ? "true" : "false");
}

static void _set_subscribe_callbacks(struct mosquitto* mosq)
{
  mosquitto_subscribe_v5_callback_set(mosq, on_subscribe);
  mosquitto_message_v5_callback_set(mosq, on_message);
}

static void _set_publish_callbacks(struct mosquitto* mosq)
{
  mosquitto_publish_v5_callback_set(mosq, on_publish);
}

void on_mosquitto_log(struct mosquitto* mosq, void* obj, int level, const char* str)
{
#ifndef LOG_ALL_MOSQUITTO
  if (strstr(str, "PINGREQ") != NULL || strstr(str, "PINGRESP") != NULL)
  {
    printf("%s\n", str);
  }
#else
  {
    char* log_level_str;
    switch (level)
    {
      case MOSQ_LOG_DEBUG:
        log_level_str = "DEBUG";
        break;
      case MOSQ_LOG_INFO:
        log_level_str = "INFO";
        break;
      case MOSQ_LOG_NOTICE:
        log_level_str = "NOTICE";
        break;
      case MOSQ_LOG_WARNING:
        log_level_str = "WARNING";
        break;
      case MOSQ_LOG_ERR:
        log_level_str = "ERROR";
        break;
      default:
        log_level_str = "";
        break;
    }
    printf("Mosquitto log: [%s] %s\n", log_level_str, str);
  }
#endif
}

struct mosquitto* mqtt_client_init(
    bool publish,
    char* env_file,
    void (*on_connect_with_subscribe)(
        struct mosquitto*,
        void*,
        int,
        int,
        const mosquitto_property* props),
    mqtt_client_obj* obj)
{
  signal(SIGINT, sig_handler);

  struct mosquitto* mosq = NULL;
  mqtt_client_connection_settings* connection_settings
      = calloc(1, sizeof(mqtt_client_connection_settings));
  bool subscribe = on_connect_with_subscribe != NULL;

  /* Get environment variables for connection settings */
  mqtt_client_read_env_file(env_file);
  mqtt_client_set_connection_settings(connection_settings);

  obj->key_file_password = connection_settings->key_file_password;
  obj->hostname = connection_settings->hostname;
  obj->keep_alive_in_seconds = connection_settings->keep_alive_in_seconds;
  obj->tcp_port = connection_settings->tcp_port;
  obj->client_id = connection_settings->client_id;

  /* Required before calling other mosquitto functions */
  RETURN_IF_FAILED(mosquitto_lib_init());

  /* Create a new client instance.
   * id = NULL -> ask the broker to generate a client id for us
   * clean session = true -> the broker should remove old sessions when we connect
   * obj = NULL -> we aren't passing any of our private data for callbacks
   */
  mosq = mosquitto_new(connection_settings->client_id, connection_settings->clean_session, obj);

  if (mosq == NULL)
  {
    printf("Error: Out of memory.\n");
    return NULL;
  }

  mosquitto_log_callback_set(mosq, on_mosquitto_log);

  RETURN_IF_FAILED(mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, obj->mqtt_version));

  /*callbacks */
  mosquitto_connect_v5_callback_set(mosq, on_connect_with_subscribe ?: on_connect);
  mosquitto_disconnect_v5_callback_set(mosq, on_disconnect);

  if (subscribe)
  {
    _set_subscribe_callbacks(mosq);
  }

  if (publish)
  {
    _set_publish_callbacks(mosq);
  }

  int result;

  if (connection_settings->username)
  {
    RETURN_IF_FAILED(mosquitto_username_pw_set(
        mosq, connection_settings->username, connection_settings->password));
  }

  if (connection_settings->use_TLS)
  {
    /* Need ca_file for mosquitto broker, but ca_path for event grid - fine for the unneeded one to
     * be null */
    RETURN_IF_FAILED(mosquitto_tls_set(
        mosq,
        connection_settings->ca_file,
        connection_settings->ca_path,
        connection_settings->cert_file,
        connection_settings->key_file,
        connection_settings->key_file_password != NULL ? key_file_password_callback : NULL));
  }

  free(connection_settings);

  return mosq;
}
