// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
// clang-format off
// cmocka has to come after stddef.h
#include <cmocka.h>
// clang-format on

#include "mqtt_client_test.h"

#define assert_bool_equal(expected, actual) assert_int_equal(expected, actual)

static const char* invalid_env_var = "cat";

// valid non-default values for all connection settings
static const char* valid_host_name = "test_host_name";

static const int valid_tcp_port = 1234;
static const char* valid_tcp_port_str = "1234";
static const bool valid_use_TLS = false;
static const char* valid_use_TLS_str = "false";
static const bool valid_clean_session = false;
static const char* valid_clean_session_str = "false";
static const int valid_keep_alive_in_seconds = 60;
static const char* valid_keep_alive_in_seconds_str = "60";
static const char* valid_client_id = "test_client_id";
static const char* valid_username = "test_username";
static const char* valid_password = "test_password";
static const char* valid_ca_file = "test_ca_file";
static const char* valid_cert_file = "test_cert_file";
static const char* valid_key_file = "test_key_file";
static const char* valid_key_file_password = "test_key_file_password";

static int setup(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  test_state->connection_settings = calloc(1, sizeof(mqtt_client_connection_settings));
  clearenv();

  return 0;
}

static int teardown(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  free(test_state->connection_settings);

  return 0;
}

// Test failure if required char environment variable is not defined
static void test_set_char_connection_setting_env_var_not_defined_failure(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  assert_false(set_char_connection_setting(&connection_settings->hostname, "MQTT_HOST_NAME", true));
  assert_null(connection_settings->hostname);
}

// Test successful setting of a char environment variable
static void test_set_char_connection_setting_sucess(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  setenv("MQTT_HOST_NAME", valid_host_name, 1);
  assert_true(set_char_connection_setting(&connection_settings->hostname, "MQTT_HOST_NAME", true));
  assert_string_equal(connection_settings->hostname, valid_host_name);
}

// Test successful setting of an int environment variable's default value
static void test_set_int_connection_setting_default_value_sucess(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  assert_true(set_int_connection_setting(
      &connection_settings->tcp_port, "MQTT_TCP_PORT", DEFAULT_TCP_PORT));
  assert_int_equal(connection_settings->tcp_port, DEFAULT_TCP_PORT);
}

// Test successful setting of an int environment variable
static void test_set_int_connection_setting_sucess(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  setenv("MQTT_TCP_PORT", valid_tcp_port_str, 1);
  assert_true(set_int_connection_setting(
      &connection_settings->tcp_port, "MQTT_TCP_PORT", DEFAULT_TCP_PORT));
  assert_int_equal(connection_settings->tcp_port, valid_tcp_port);
}

// Test failure if invalid int environment variable is defined
static void test_set_int_connection_setting_invalid_int_failure(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  setenv("MQTT_TCP_PORT", invalid_env_var, 1);
  assert_false(set_int_connection_setting(
      &connection_settings->tcp_port, "MQTT_TCP_PORT", DEFAULT_TCP_PORT));
  assert_null(connection_settings->tcp_port);
}

// Test successful setting of an bool environment variable's default value
static void test_set_bool_connection_setting_default_value_sucess(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  assert_true(set_bool_connection_setting(
      &connection_settings->clean_session, "MQTT_CLEAN_SESSION", DEFAULT_CLEAN_SESSION));
  assert_bool_equal(connection_settings->clean_session, DEFAULT_CLEAN_SESSION);
}

// Test successful setting of an bool environment variable
static void test_set_bool_connection_setting_sucess(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  setenv("MQTT_CLEAN_SESSION", valid_clean_session_str, 1);
  assert_true(set_bool_connection_setting(
      &connection_settings->clean_session, "MQTT_CLEAN_SESSION", DEFAULT_CLEAN_SESSION));
  assert_bool_equal(connection_settings->clean_session, valid_clean_session);
}

// Test failure if invalid bool environment variable is defined
static void test_set_bool_connection_setting_invalid_bool_failure(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  setenv("MQTT_CLEAN_SESSION", invalid_env_var, 1);
  assert_false(set_bool_connection_setting(
      &connection_settings->clean_session, "MQTT_CLEAN_SESSION", DEFAULT_CLEAN_SESSION));
  assert_null(connection_settings->clean_session);
}

// Test failed setting of all connection settings if no environment variables are defined
static void test_mqtt_client_set_connection_settings_no_env_vars_failure(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  assert_false(mqtt_client_set_connection_settings(connection_settings));
}

// Test minimum successful setting of all connection settings
static void test_mqtt_client_set_connection_settings_min_sucess(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  setenv("MQTT_HOST_NAME", valid_host_name, 1);

  assert_true(mqtt_client_set_connection_settings(connection_settings));

  assert_string_equal(connection_settings->hostname, valid_host_name);
  assert_int_equal(connection_settings->tcp_port, DEFAULT_TCP_PORT);
  assert_bool_equal(connection_settings->use_TLS, DEFAULT_USE_TLS);
  assert_bool_equal(connection_settings->clean_session, DEFAULT_CLEAN_SESSION);
  assert_int_equal(connection_settings->keep_alive_in_seconds, DEFAULT_KEEP_ALIVE_IN_SECONDS);
  assert_null(connection_settings->client_id);
  assert_null(connection_settings->username);
  assert_null(connection_settings->password);
  assert_null(connection_settings->ca_file);
  assert_null(connection_settings->cert_file);
  assert_null(connection_settings->key_file);
  assert_null(connection_settings->key_file_password);
}

// Test setting all connection settings
static void test_mqtt_client_set_connection_settings_max_sucess(void** state)
{
  mqtt_client_test_state* test_state = (mqtt_client_test_state*)state;
  mqtt_client_connection_settings* connection_settings = test_state->connection_settings;

  setenv("MQTT_HOST_NAME", valid_host_name, 1);
  setenv("MQTT_TCP_PORT", valid_tcp_port_str, 1);
  setenv("MQTT_USE_TLS", valid_use_TLS_str, 1);
  setenv("MQTT_CLEAN_SESSION", valid_clean_session_str, 1);
  setenv("MQTT_KEEP_ALIVE_IN_SECONDS", valid_keep_alive_in_seconds_str, 1);
  setenv("MQTT_CLIENT_ID", valid_client_id, 1);
  setenv("MQTT_USERNAME", valid_username, 1);
  setenv("MQTT_PASSWORD", valid_password, 1);
  setenv("MQTT_CA_FILE", valid_ca_file, 1);
  setenv("MQTT_CERT_FILE", valid_cert_file, 1);
  setenv("MQTT_KEY_FILE", valid_key_file, 1);
  setenv("MQTT_KEY_FILE_PASSWORD", valid_key_file_password, 1);

  assert_true(mqtt_client_set_connection_settings(connection_settings));

  assert_string_equal(connection_settings->hostname, valid_host_name);
  assert_int_equal(connection_settings->tcp_port, valid_tcp_port);
  assert_bool_equal(connection_settings->use_TLS, valid_use_TLS);
  assert_bool_equal(connection_settings->clean_session, valid_clean_session);
  assert_int_equal(connection_settings->keep_alive_in_seconds, valid_keep_alive_in_seconds);
  assert_string_equal(connection_settings->client_id, valid_client_id);
  assert_string_equal(connection_settings->username, valid_username);
  assert_string_equal(connection_settings->password, valid_password);
  assert_string_equal(connection_settings->ca_file, valid_ca_file);
  assert_string_equal(connection_settings->cert_file, valid_cert_file);
  assert_string_equal(connection_settings->key_file, valid_key_file);
  assert_string_equal(connection_settings->key_file_password, valid_key_file_password);
}

int test_mqtt_client()
{
  const struct CMUnitTest tests[]
      = { // char connection settings tests
          cmocka_unit_test_setup_teardown(
              test_set_char_connection_setting_env_var_not_defined_failure, setup, teardown),
          cmocka_unit_test_setup_teardown(test_set_char_connection_setting_sucess, setup, teardown),
          // int connection settings tests
          cmocka_unit_test_setup_teardown(
              test_set_int_connection_setting_default_value_sucess, setup, teardown),
          cmocka_unit_test_setup_teardown(test_set_int_connection_setting_sucess, setup, teardown),
          cmocka_unit_test_setup_teardown(
              test_set_int_connection_setting_invalid_int_failure, setup, teardown),
          // bool connection settings tests
          cmocka_unit_test_setup_teardown(
              test_set_bool_connection_setting_default_value_sucess, setup, teardown),
          cmocka_unit_test_setup_teardown(test_set_bool_connection_setting_sucess, setup, teardown),
          cmocka_unit_test_setup_teardown(
              test_set_bool_connection_setting_invalid_bool_failure, setup, teardown),
          // set all connection settings tests
          cmocka_unit_test_setup_teardown(
              test_mqtt_client_set_connection_settings_no_env_vars_failure, setup, teardown),
          cmocka_unit_test_setup_teardown(
              test_mqtt_client_set_connection_settings_min_sucess, setup, teardown),
          cmocka_unit_test_setup_teardown(
              test_mqtt_client_set_connection_settings_max_sucess, setup, teardown)
        };
  return cmocka_run_group_tests_name("mqtt_client", tests, NULL, NULL);
}