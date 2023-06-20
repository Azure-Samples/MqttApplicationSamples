// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// clang-format off
// cmocka has to come after stddef.h
#include <cmocka.h>
// clang-format on

#include "json_handler_test.h"

#define MAX_PAYLOAD_LENGTH 60

// Init sets type to "Point" and coordinates to (0, 0)
static void test_geojson_point_init_success(void** state)
{
  geojson_point json_point = geojson_point_init();

  assert_string_equal(json_point.type, "Point");
  assert_float_equal(json_point.coordinates.x, 0, 0.1);
  assert_float_equal(json_point.coordinates.y, 0, 0.1);
}

// Init sets payload length to 0, payload to empty string, and saves buffer size in
// max_payload_length
static void mosquitto_payload_init_success(void** state)
{
  mosquitto_payload mosq_payload = mosquitto_payload_init(MAX_PAYLOAD_LENGTH);

  assert_int_equal(mosq_payload.payload_length, 0);
  assert_int_equal(strlen(mosq_payload.payload), 0);
  assert_int_equal(mosq_payload.max_payload_length, MAX_PAYLOAD_LENGTH);

  mosquitto_payload_destroy(&mosq_payload);
}

// Destroy clears payload and payload length
static void mosquitto_payload_destroy_success(void** state)
{
  mosquitto_payload mosq_payload = mosquitto_payload_init(MAX_PAYLOAD_LENGTH);

  // set values to make sure they get properly cleared
  char* test_payload = "test";
  size_t test_payload_length = strlen(test_payload);
  strcpy(mosq_payload.payload, test_payload);
  mosq_payload.payload_length = test_payload_length;

  assert_string_equal(mosq_payload.payload, test_payload);
  assert_int_equal(mosq_payload.payload_length, test_payload_length);

  mosquitto_payload_destroy(&mosq_payload);
  assert_null(mosq_payload.payload);
  assert_int_equal(mosq_payload.payload_length, 0);
}

// min length payload
static void test_geojson_point_to_mosquitto_payload_min_length_success(void** state)
{
  geojson_point json_point = geojson_point_init();
  mosquitto_payload mosq_payload = mosquitto_payload_init(MAX_PAYLOAD_LENGTH);

  assert_int_equal(0, geojson_point_to_mosquitto_payload(json_point, &mosq_payload));

  assert_string_equal(
      mosq_payload.payload, "{\"type\":\"Point\",\"coordinates\":[0.000000,0.000000]}");

  mosquitto_payload_destroy(&mosq_payload);
}

// max length payload
static void test_geojson_point_to_mosquitto_payload_max_length_success(void** state)
{
  geojson_point json_point = geojson_point_init();
  mosquitto_payload mosq_payload = mosquitto_payload_init(MAX_PAYLOAD_LENGTH);
  geojson_point_set_coordinates(&json_point, -83.55107123423, -36.169784234234);

  assert_int_equal(0, geojson_point_to_mosquitto_payload(json_point, &mosq_payload));

  assert_string_equal(
      mosq_payload.payload, "{\"type\":\"Point\",\"coordinates\":[-83.551071,-36.169784]}");

  mosquitto_payload_destroy(&mosq_payload);
}

// NULL type
static void test_geojson_point_to_mosquitto_payload_null_type_fail(void** state)
{

  mosquitto_payload mosq_payload = mosquitto_payload_init(MAX_PAYLOAD_LENGTH);
  geojson_point json_point
      = { .type = NULL, .coordinates = (geojson_coordinates){ .x = 5, .y = 3 } };
  assert_int_equal(-1, geojson_point_to_mosquitto_payload(json_point, &mosq_payload));
  assert_int_equal(mosq_payload.payload_length, 0);
  assert_int_equal(strlen(mosq_payload.payload), 0);

  mosquitto_payload_destroy(&mosq_payload);
}

// output payload buffer is null
static void test_geojson_point_to_mosquitto_payload_output_null_fail(void** state)
{

  mosquitto_payload mosq_payload;
  mosq_payload.payload = NULL;
  geojson_point json_point = geojson_point_init();
  assert_int_equal(-1, geojson_point_to_mosquitto_payload(json_point, &mosq_payload));
}

// output payload buffer is too small
static void test_geojson_point_to_mosquitto_payload_output_buffer_too_small_fail(void** state)
{

  mosquitto_payload mosq_payload = mosquitto_payload_init(5);
  geojson_point json_point = geojson_point_init();
  assert_int_equal(-1, geojson_point_to_mosquitto_payload(json_point, &mosq_payload));
  assert_int_equal(mosq_payload.payload_length, 0);
  assert_int_equal(strlen(mosq_payload.payload), 0);

  mosquitto_payload_destroy(&mosq_payload);
}

static void test_geojson_point_set_coordinates_sucess(void** state)
{
  geojson_point json_point = geojson_point_init();

  assert_float_equal(json_point.coordinates.x, 0, 0.1);
  assert_float_equal(json_point.coordinates.y, 0, 0.1);

  geojson_point_set_coordinates(&json_point, -83.551071, -36.169784);

  assert_float_equal(json_point.coordinates.x, -83.551071, 0.000001);
  assert_float_equal(json_point.coordinates.y, -36.169784, 0.000001);
}

static void test_mosquitto_payload_to_geojson_point_min_payload_success(void** state)
{
  struct mosquitto_message message;
  message.payload = "{\"type\":\"Point\",\"coordinates\":[0.000000,0.000000]}";
  geojson_point json_point = geojson_point_init();

  assert_int_equal(mosquitto_payload_to_geojson_point(&message, &json_point), 0);
  assert_string_equal(json_point.type, "Point");
  assert_float_equal(json_point.coordinates.x, 0, 0.1);
  assert_float_equal(json_point.coordinates.y, 0, 0.1);
}

static void test_mosquitto_payload_to_geojson_point_max_payload_success(void** state)
{
  struct mosquitto_message message;
  message.payload = "{\"type\":\"Point\",\"coordinates\":[-83.551071,-36.169784]}";
  geojson_point json_point = geojson_point_init();

  assert_int_equal(mosquitto_payload_to_geojson_point(&message, &json_point), 0);
  assert_string_equal(json_point.type, "Point");
  assert_float_equal(json_point.coordinates.x, -83.551071, 0.000001);
  assert_float_equal(json_point.coordinates.y, -36.169784, 0.000001);
}

// both params NULL
static void test_mosquitto_payload_to_geojson_point_null_params_fail(void** state)
{
  assert_int_equal(mosquitto_payload_to_geojson_point(NULL, NULL), -1);
}

// null message
static void test_mosquitto_payload_to_geojson_point_null_message_fail(void** state)
{
  geojson_point json_point = geojson_point_init();
  assert_int_equal(mosquitto_payload_to_geojson_point(NULL, &json_point), -1);
}

// null json point
static void test_mosquitto_payload_to_geojson_point_null_json_point_fail(void** state)
{
  struct mosquitto_message message;
  assert_int_equal(mosquitto_payload_to_geojson_point(&message, NULL), -1);
}

// empty json in message payload
static void test_mosquitto_payload_to_geojson_point_empty_json_fail(void** state)
{
  geojson_point json_point = geojson_point_init();
  struct mosquitto_message message;
  message.payload = "";
  assert_int_equal(mosquitto_payload_to_geojson_point(&message, &json_point), -1);
}

// not geojson
static void test_mosquitto_payload_to_geojson_point_not_geojson_fail(void** state)
{
  geojson_point json_point = geojson_point_init();
  struct mosquitto_message message;
  message.payload = "{\"name\":\"Valerie\",\"shirtColor\":\"blue\"}";
  assert_int_equal(mosquitto_payload_to_geojson_point(&message, &json_point), -1);
}

// not a Point
static void test_mosquitto_payload_to_geojson_point_not_point_fail(void** state)
{
  geojson_point json_point = geojson_point_init();
  struct mosquitto_message message;
  message.payload = "{\"type\":\"LineString\",\"coordinates\":[[100.0, 0.0],[101.0, 1.0]]}";
  assert_int_equal(mosquitto_payload_to_geojson_point(&message, &json_point), -1);
}

// missing coordinates
static void test_mosquitto_payload_to_geojson_point_missing_coordinates_fail(void** state)
{
  geojson_point json_point = geojson_point_init();
  struct mosquitto_message message;
  message.payload = "{\"type\":\"Point\"}";
  assert_int_equal(mosquitto_payload_to_geojson_point(&message, &json_point), -1);
}

int test_json_handler()
{
  const struct CMUnitTest tests[]
      = { cmocka_unit_test(test_geojson_point_init_success),
          cmocka_unit_test(mosquitto_payload_init_success),
          cmocka_unit_test(mosquitto_payload_destroy_success),
          cmocka_unit_test(test_geojson_point_to_mosquitto_payload_min_length_success),
          cmocka_unit_test(test_geojson_point_to_mosquitto_payload_max_length_success),
          cmocka_unit_test(test_geojson_point_to_mosquitto_payload_null_type_fail),
          cmocka_unit_test(test_geojson_point_to_mosquitto_payload_output_null_fail),
          cmocka_unit_test(test_geojson_point_to_mosquitto_payload_output_buffer_too_small_fail),
          cmocka_unit_test(test_geojson_point_set_coordinates_sucess),
          cmocka_unit_test(test_mosquitto_payload_to_geojson_point_min_payload_success),
          cmocka_unit_test(test_mosquitto_payload_to_geojson_point_max_payload_success),
          cmocka_unit_test(test_mosquitto_payload_to_geojson_point_null_params_fail),
          cmocka_unit_test(test_mosquitto_payload_to_geojson_point_null_message_fail),
          cmocka_unit_test(test_mosquitto_payload_to_geojson_point_null_json_point_fail),
          cmocka_unit_test(test_mosquitto_payload_to_geojson_point_empty_json_fail),
          cmocka_unit_test(test_mosquitto_payload_to_geojson_point_not_geojson_fail),
          cmocka_unit_test(test_mosquitto_payload_to_geojson_point_not_point_fail),
          cmocka_unit_test(test_mosquitto_payload_to_geojson_point_missing_coordinates_fail)
        };
  return cmocka_run_group_tests_name("json_handler", tests, NULL, NULL);
}
