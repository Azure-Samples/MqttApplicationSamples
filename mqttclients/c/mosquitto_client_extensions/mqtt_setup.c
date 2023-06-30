/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "mosquitto.h"
#include "mqtt_callbacks.h"
#include "mqtt_setup.h"

// A certificate path (any string) is required when configuring mosquitto to use OS certificates
// when use_TLS is true and you're not using a ca file.
#define REQUIRED_TLS_SET_CERT_PATH "L"

volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
  (void)_;
  keep_running = 0;
}

#define MQTT_RETURN_IF_FAILED(rc)                                        \
  do                                                                     \
  {                                                                      \
    enum mosq_err_t const mosq_result = (rc);                            \
    if (mosq_result != MOSQ_ERR_SUCCESS)                                 \
    {                                                                    \
      if (mosq != NULL)                                                  \
      {                                                                  \
        mosquitto_destroy(mosq);                                         \
      }                                                                  \
      LOG_ERROR("Mosquitto Error: %s", mosquitto_strerror(mosq_result)); \
      return NULL;                                                       \
    }                                                                    \
  } while (0)

#define RETURN_FALSE_IF_FAILED(rc) \
  do                               \
  {                                \
    if (rc == false)               \
    {                              \
      return false;                \
    }                              \
  } while (0)

void mqtt_client_read_env_file(char* file_path)
{
  /* If there was no env file_path passed in, look for a .env file in the current directory. */
  if (file_path == NULL)
  {
    file_path = ".env";
  }

  LOG_INFO(APP_LOG_TAG, "Loading environment variables from %s", file_path);

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
    LOG_WARNING("Cannot open env file. Sample will try to use environment variables.");
  }
}

/**
 * @brief Sets a string connection setting from environment variables.
 * @param connection_setting The connection setting to set.
 * @param env_name The name of the environment variable to read.
 * @param fail_not_defined Whether the function should fail if the environment variable isn't set.
 *
 * @return true if connection setting successfully set, false if invalid.
 */
bool set_char_connection_setting(
    char** connection_setting,
    const char* env_name,
    bool fail_not_defined)
{
  char* env_value = getenv(env_name);
  if (env_value == NULL && fail_not_defined)
  {
    LOG_ERROR("Environment variable %s is required but not set.", env_name);
    return false;
  }
  *connection_setting = env_value;
  printf("\t%s = %s\n", env_name, *connection_setting);

  return true;
}

/**
 * @brief Sets an int connection setting from environment variables.
 * @param connection_setting The connection setting to set.
 * @param env_name The name of the environment variable to read.
 * @param default_value The default value to use if the environment variable isn't set.
 *
 * @return true if connection setting successfully set, false if environment variable isn't an int.
 */
bool set_int_connection_setting(int* connection_setting, char* env_name, int default_value)
{
  char* env_value = getenv(env_name);
  if (env_value == NULL)
  {
    *connection_setting = default_value;
    printf("\t%s = %d (Default value)\n", env_name, *connection_setting);
  }
  else
  {
    int env_int_value = atoi(env_value);
    if (env_int_value == 0 && strcmp(env_value, "0") != 0)
    {
      LOG_ERROR("Environment variable %s (value: %s) is not a valid integer.", env_name, env_value);
      return false;
    }
    else
    {
      *connection_setting = env_int_value;
      printf("\t%s = %d\n", env_name, *connection_setting);
    }
  }
  return true;
}

/**
 * @brief Sets a bool connection setting from environment variables.
 * @param connection_setting The connection setting to set.
 * @param env_name The name of the environment variable to read.
 * @param default_value The default value to use if the environment variable isn't set.
 *
 * @return true if connection setting successfully set, false if environment variable isn't a bool.
 */
bool set_bool_connection_setting(bool* connection_setting, char* env_name, bool default_value)
{
  char* env_value = getenv(env_name);
  if (env_value == NULL)
  {
    *connection_setting = default_value;
    printf("\t%s = %s (Default value)\n", env_name, *connection_setting ? "true" : "false");
    return true;
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
      LOG_ERROR("Environment variable %s (value: %s) is not a valid boolean.", env_name, env_value);
      return false;
    }
    printf("\t%s = %s\n", env_name, *connection_setting ? "true" : "false");
    return true;
  }
}

/**
 * @brief Set a connection settings from environment variables.
 * @param connection_settings The connection settings struct to write to.
 *
 * @return true if successful, or false if any environment variables are invalid.
 */
bool mqtt_client_set_connection_settings(mqtt_client_connection_settings* connection_settings)
{
  RETURN_FALSE_IF_FAILED(
      set_char_connection_setting(&connection_settings->hostname, "MQTT_HOST_NAME", true));
  RETURN_FALSE_IF_FAILED(set_int_connection_setting(
      &connection_settings->tcp_port, "MQTT_TCP_PORT", DEFAULT_TCP_PORT));
  RETURN_FALSE_IF_FAILED(
      set_bool_connection_setting(&connection_settings->use_TLS, "MQTT_USE_TLS", DEFAULT_USE_TLS));
  RETURN_FALSE_IF_FAILED(set_bool_connection_setting(
      &connection_settings->clean_session, "MQTT_CLEAN_SESSION", DEFAULT_CLEAN_SESSION));
  RETURN_FALSE_IF_FAILED(set_int_connection_setting(
      &connection_settings->keep_alive_in_seconds,
      "MQTT_KEEP_ALIVE_IN_SECONDS",
      DEFAULT_KEEP_ALIVE_IN_SECONDS));
  RETURN_FALSE_IF_FAILED(
      set_char_connection_setting(&connection_settings->client_id, "MQTT_CLIENT_ID", false));
  RETURN_FALSE_IF_FAILED(
      set_char_connection_setting(&connection_settings->username, "MQTT_USERNAME", false));
  RETURN_FALSE_IF_FAILED(
      set_char_connection_setting(&connection_settings->password, "MQTT_PASSWORD", false));
  RETURN_FALSE_IF_FAILED(
      set_char_connection_setting(&connection_settings->ca_file, "MQTT_CA_FILE", false));
  RETURN_FALSE_IF_FAILED(
      set_char_connection_setting(&connection_settings->cert_file, "MQTT_CERT_FILE", false));
  RETURN_FALSE_IF_FAILED(
      set_char_connection_setting(&connection_settings->key_file, "MQTT_KEY_FILE", false));
  RETURN_FALSE_IF_FAILED(set_char_connection_setting(
      &connection_settings->key_file_password, "MQTT_KEY_FILE_PASSWORD", false));

  return true;
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
    LOG_INFO(MOSQUITTO_LOG_TAG, "%s", str);
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
    LOG_INFO(MOSQUITTO_LOG_TAG, "[%s] %s", log_level_str, str);
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
  mqtt_client_connection_settings connection_settings;
  bool subscribe = on_connect_with_subscribe != NULL;

  /* Get environment variables for connection settings */
  mqtt_client_read_env_file(env_file);
  if (!mqtt_client_set_connection_settings(&connection_settings))
  {
    LOG_ERROR("Failed to set connection settings.");
    return NULL;
  }

  obj->hostname = connection_settings.hostname;
  obj->keep_alive_in_seconds = connection_settings.keep_alive_in_seconds;
  obj->tcp_port = connection_settings.tcp_port;
  obj->client_id = connection_settings.client_id;

  /* Required before calling other mosquitto functions */
  MQTT_RETURN_IF_FAILED(mosquitto_lib_init());

  /* Create a new client instance.
   * id = NULL -> ask the broker to generate a client id for us
   * clean session = true -> the broker should remove old sessions when we connect
   * obj = NULL -> we aren't passing any of our private data for callbacks
   */
  mosq = mosquitto_new(connection_settings.client_id, connection_settings.clean_session, obj);

  if (mosq == NULL)
  {
    LOG_ERROR("Out of memory.");
    return NULL;
  }

  mosquitto_log_callback_set(mosq, on_mosquitto_log);

  printf(
      "\tMQTT_VERSION = %s\n",
      obj->mqtt_version == MQTT_PROTOCOL_V5
          ? "MQTT_PROTOCOL_V5"
          : obj->mqtt_version == MQTT_PROTOCOL_V311 ? "MQTT_PROTOCOL_V311" : "UNKNOWN");
  MQTT_RETURN_IF_FAILED(mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, obj->mqtt_version));

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

  if (connection_settings.username)
  {
    MQTT_RETURN_IF_FAILED(mosquitto_username_pw_set(
        mosq, connection_settings.username, connection_settings.password));
  }

  if (connection_settings.use_TLS)
  {
    bool use_OS_certs = connection_settings.ca_file == NULL;
    if (use_OS_certs)
    {
      MQTT_RETURN_IF_FAILED(mosquitto_int_option(mosq, MOSQ_OPT_TLS_USE_OS_CERTS, true));
    }
    MQTT_RETURN_IF_FAILED(mosquitto_tls_set(
        mosq,
        connection_settings.ca_file,
        use_OS_certs ? REQUIRED_TLS_SET_CERT_PATH : NULL,
        connection_settings.cert_file,
        connection_settings.key_file,
        NULL));
  }

  return mosq;
}
