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
      free(connection_settings);                 \
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

int set_char_connection_setting(char** connection_setting, char* env_name, bool required)
{
  char* env_value = getenv(env_name);
  if (env_value == NULL && required)
  {
    printf("Environment variable %s is required but not set.\n", env_name);
    return 1;
  }
  *connection_setting = env_value;
  printf("%s = %s\n", env_name, *connection_setting);

  return 0;
}

int set_int_connection_setting(int* connection_setting, char* env_name, int default_value)
{
  char* env_value = getenv(env_name);
  if (env_value == NULL)
  {
    *connection_setting = default_value;
    printf("%s = %d (Default value)\n", env_name, *connection_setting);
  }
  else
  {
    int env_int_value = atoi(env_value);
    if (env_int_value == 0 && strcmp(env_value, "0") != 0)
    {
      printf("Environment variable %s (value: %s) is not a valid integer.\n", env_name, env_value);
      return 1;
    }
    else
    {
      *connection_setting = env_int_value;
      printf("%s = %d\n", env_name, *connection_setting);
    }
  }
  return 0;
}

int set_bool_connection_setting(bool* connection_setting, char* env_name, bool default_value)
{
  char* env_value = getenv(env_name);
  if (env_value == NULL)
  {
    *connection_setting = default_value;
    printf("%s = %s (Default value)\n", env_name, *connection_setting ? "true" : "false");
    return 0;
  }
  else
  {
    if (strcmp(env_value, "true") == 0)
    {
      *connection_setting = true;
    }
    else if (strcmp(env_value, "false") == 0)
    {
      *connection_setting = false;
    }
    else
    {
      printf("Environment variable %s (value: %s) is not a valid boolean.\n", env_name, env_value);
      return 1;
    }
    printf("%s = %s\n", env_name, *connection_setting ? "true" : "false");
    return 0;
  }
}

int mqtt_client_set_connection_settings(mqtt_client_connection_settings* connection_settings)
{
  int failures = 0;

  failures += set_char_connection_setting(&connection_settings->hostname, "MQTT_HOST_NAME", true);
  failures += set_int_connection_setting(&connection_settings->tcp_port, "MQTT_TCP_PORT", 8883);
  failures += set_bool_connection_setting(&connection_settings->use_TLS, "MQTT_USE_TLS", true);
  failures += set_bool_connection_setting(
      &connection_settings->clean_session, "MQTT_CLEAN_SESSION", true);
  failures += set_int_connection_setting(
      &connection_settings->keep_alive_in_seconds, "MQTT_KEEP_ALIVE_IN_SECONDS", 30);
  failures += set_char_connection_setting(&connection_settings->client_id, "MQTT_CLIENT_ID", false);
  failures += set_char_connection_setting(&connection_settings->username, "MQTT_USERNAME", false);
  failures += set_char_connection_setting(&connection_settings->password, "MQTT_PASSWORD", false);
  failures += set_char_connection_setting(&connection_settings->ca_file, "MQTT_CA_FILE", false);
  failures += set_char_connection_setting(&connection_settings->ca_path, "MQTT_CA_PATH", false);
  failures += set_char_connection_setting(&connection_settings->cert_file, "MQTT_CERT_FILE", false);
  failures += set_char_connection_setting(&connection_settings->key_file, "MQTT_KEY_FILE", false);
  failures += set_char_connection_setting(
      &connection_settings->key_file_password, "MQTT_KEY_FILE_PASSWORD", false);

  return failures;
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
  if (level == MOSQ_LOG_ERR || strstr(str, "PINGREQ") != NULL || strstr(str, "PINGRESP") != NULL)
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
  if (mqtt_client_set_connection_settings(connection_settings) != 0)
  {
    printf("Error: Failed to set connection settings.\n");
    return NULL;
  }

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

  printf(
      "MQTT_VERSION = %s\n",
      obj->mqtt_version == MQTT_PROTOCOL_V5
          ? "MQTT_PROTOCOL_V5"
          : obj->mqtt_version == MQTT_PROTOCOL_V311 ? "MQTT_PROTOCOL_V311" : "UNKNOWN");
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
        NULL));
  }

  return mosq;
}
