/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt_callbacks.h"
#include "mqtt_setup.h"
#include <mosquitto.h>

void mqtt_client_read_env_file(char* file_path)
{
  /* TODO: think about whether this is an issue if we specify the env variables in launch.json, but
   * have a .env that then overwrites them */
  /* If there was no env file_path passed in, look for a .env file in the current directory. */
  if (file_path == NULL)
  {
    file_path = ".env";
  }

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
      printf("Setting %s = %s\n", env_name, env_value);
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
  connection_settings->hostname = getenv("HOSTNAME");
  connection_settings->tcp_port = atoi(getenv("TCP_PORT") ?: "8883");
  connection_settings->client_id = getenv("CLIENT_ID");
  connection_settings->ca_file = getenv("CA_FILE");
  /* TODO: this might not work because we could keep an env var from a previous session */
  connection_settings->ca_path
      = getenv("CA_PATH") ?: connection_settings->ca_file ? NULL : "/etc/ssl/certs";
  connection_settings->cert_file = getenv("CERT_FILE");
  connection_settings->key_file = getenv("KEY_FILE");
  connection_settings->key_file_password = getenv("KEY_FILE_PASSWORD");
  connection_settings->qos = atoi(getenv("QOS") ?: "1");
  connection_settings->keep_alive_in_seconds = atoi(getenv("KEEP_ALIVE_IN_SECONDS") ?: "30");
  char* use_TLS = getenv("USE_TLS");
  connection_settings->use_TLS = (use_TLS != NULL && strcmp(use_TLS, "false") == 0)
      ? false
      : true; /* TODO: figure out "cat" case */
  connection_settings->mqtt_version = atoi(getenv("MQTT_VERSION") ?: "4");
  connection_settings->username = getenv("USERNAME");
  connection_settings->password = getenv("PASSWORD");
  char* clean_session = getenv("CLEAN_SESSION");
  connection_settings->clean_session
      = (clean_session != NULL && strcmp(clean_session, "false") == 0)
      ? false
      : true; /* TODO: figure out "cat" case */
}

static void _set_subscribe_callbacks(struct mosquitto* mosq)
{
  mosquitto_connect_v5_callback_set(mosq, on_connect_with_subscribe);
  mosquitto_subscribe_v5_callback_set(mosq, on_subscribe);
  mosquitto_message_v5_callback_set(mosq, on_message);
}

static void _set_publish_callbacks(struct mosquitto* mosq)
{
  mosquitto_publish_v5_callback_set(mosq, on_publish);
}

void on_mosquitto_log(struct mosquitto* mosq, void* obj, int level, const char* str)
{
  fprintf(stderr, "Mosquitto log: [%d] %s\n", level, str);
}

struct mosquitto* mqtt_client_init(
    bool publish,
    char* env_file,
    mqtt_client_connection_settings* connection_settings)
{
  struct mosquitto* mosq = NULL;
  bool subscribe = false;

  if (connection_settings->sub_topic)
  {
    setenv("SUB_TOPIC", connection_settings->sub_topic, 1);
    subscribe = true;
  }

  /* Get environment variables for connection settings */
  mqtt_client_read_env_file(env_file);
  mqtt_client_set_connection_settings(connection_settings);

  /* Required before calling other mosquitto functions */
  mosquitto_lib_init();

  /* Create a new client instance.
   * id = NULL -> ask the broker to generate a client id for us
   * clean session = true -> the broker should remove old sessions when we connect
   * obj = NULL -> we aren't passing any of our private data for callbacks
   */
  mosq = mosquitto_new(connection_settings->client_id, connection_settings->clean_session, NULL);

  if (mosq == NULL)
  {
    printf("Error: Out of memory.\n");
    return NULL;
  }

  /* mosquitto_log_callback_set(mosq, on_mosquitto_log); */

  mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, connection_settings->mqtt_version);

  /*callbacks */
  mosquitto_disconnect_v5_callback_set(mosq, on_disconnect);

  if (subscribe)
  {
    _set_subscribe_callbacks(mosq);
  }

  if (publish)
  {
    _set_publish_callbacks(mosq);

    /* If we're publishing and subscribing, we set the connect callback in the subscribe callbacks
     */
    if (!subscribe)
    {
      mosquitto_connect_v5_callback_set(mosq, on_connect);
    }
  }

  int result;

  if (connection_settings->username)
  {
    result = mosquitto_username_pw_set(
        mosq, connection_settings->username, connection_settings->password);

    if (result != MOSQ_ERR_SUCCESS)
    {
      mosquitto_destroy(mosq);
      printf("Error setting username/password: %s\n", mosquitto_strerror(result));
      return NULL;
    }
  }

  if (connection_settings->use_TLS)
  {
    /* Need ca_file for mosquitto broker, but ca_path for event grid - fine for the unneeded one to
     * be null */
    result = mosquitto_tls_set(
        mosq,
        connection_settings->ca_file,
        connection_settings->ca_path,
        connection_settings->cert_file,
        connection_settings->key_file,
        NULL);

    if (result != MOSQ_ERR_SUCCESS)
    {
      mosquitto_destroy(mosq);
      printf("TLS Error: %s\n", mosquitto_strerror(result));
      return NULL;
    }
  }

  return mosq;
}
