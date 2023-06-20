/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <errno.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "geo_json_handler.h"

#define RETURN_IF_NULL(x, jobj_to_free)               \
  do                                                  \
  {                                                   \
    if ((x) == NULL)                                  \
    {                                                 \
      printf("JSON Parsing Error: %s is NULL\n", #x); \
      if (jobj_to_free != NULL)                       \
      {                                               \
        json_object_put(jobj_to_free);                \
      }                                               \
      return -1;                                      \
    }                                                 \
  } while (0)

#define RETURN_IF_NON_ZERO(x)                 \
  do                                          \
  {                                           \
    if ((x) != 0)                             \
    {                                         \
      printf("JSON Parsing Error: %s\n", #x); \
      json_object_put(jobj);                  \
      return -1;                              \
    }                                         \
  } while (0)

#define RETURN_IF_NAN(x)                                      \
  do                                                          \
  {                                                           \
    errno = 0;                                                \
    x;                                                        \
    if (errno == EINVAL)                                      \
    {                                                         \
      printf("JSON Parsing Error: %s is not a number\n", #x); \
      json_object_put(jobj);                                  \
      return -1;                                              \
    }                                                         \
  } while (0)

geojson_point geojson_point_init()
{
  return (geojson_point){ .type = "Point", .coordinates = (geojson_coordinates){ .x = 0, .y = 0 } };
}

mosquitto_payload mosquitto_payload_init(int max_payload_length)
{
  return (mosquitto_payload){ .payload = calloc(1, max_payload_length),
                              .payload_length = 0,
                              .max_payload_length = max_payload_length };
}

void mosquitto_payload_destroy(mosquitto_payload* payload)
{
  if (payload->payload != NULL)
  {
    free(payload->payload);
    payload->payload = NULL;
  }
  payload->payload_length = 0;
}

void geojson_point_set_coordinates(geojson_point* pt, double x, double y)
{
  pt->coordinates.x = x;
  pt->coordinates.y = y;
}

int mosquitto_payload_to_geojson_point(
    const struct mosquitto_message* message,
    geojson_point* output)
{
  RETURN_IF_NULL(message, NULL);
  RETURN_IF_NULL(output, NULL);

  json_object* type;
  json_object* coordinates;
  json_object* jobj = json_tokener_parse(message->payload);

  RETURN_IF_NULL(type = json_object_object_get(jobj, "type"), jobj);
  RETURN_IF_NULL(output->type = (char*)json_object_get_string(type), jobj);
  if (strcmp(output->type, "Point") != 0)
  {
    printf("JSON Parsing Error: type is not Point\n");
    json_object_put(jobj);
    return -1;
  }
  RETURN_IF_NULL(coordinates = json_object_object_get(jobj, "coordinates"), jobj);
  RETURN_IF_NAN(
      output->coordinates.x = json_object_get_double(json_object_array_get_idx(coordinates, 0)));
  printf("x: %f\n", output->coordinates.x);
  RETURN_IF_NAN(
      output->coordinates.y = json_object_get_double(json_object_array_get_idx(coordinates, 1)));
  printf("y: %f\n", output->coordinates.y);
  printf("message: %s\n", message->payload);

  // decrements the reference count of the object and frees it if it reaches zero.
  json_object_put(jobj);

  return 0;
}

int geojson_point_to_mosquitto_payload(
    const geojson_point geojson_point,
    mosquitto_payload* message)
{
  RETURN_IF_NULL(geojson_point.type, NULL);
  RETURN_IF_NULL(message->payload, NULL);

  const char* payload;
  json_object* jobj = json_object_new_object();
  json_object* type = json_object_new_string(geojson_point.type);
  json_object* coordinates = json_object_new_array();

  RETURN_IF_NON_ZERO(json_c_set_serialization_double_format("%.6f", JSON_C_OPTION_THREAD));
  RETURN_IF_NON_ZERO(
      json_object_array_add(coordinates, json_object_new_double(geojson_point.coordinates.x)));
  RETURN_IF_NON_ZERO(
      json_object_array_add(coordinates, json_object_new_double(geojson_point.coordinates.y)));
  RETURN_IF_NON_ZERO(json_object_object_add(jobj, "type", type));
  RETURN_IF_NON_ZERO(json_object_object_add(jobj, "coordinates", coordinates));
  RETURN_IF_NULL(
      payload
      = json_object_to_json_string_length(jobj, JSON_C_TO_STRING_PLAIN, &message->payload_length),
      jobj);
  if (message->payload_length > message->max_payload_length)
  {
    printf("JSON Parsing Error: mosquitto payload buffer is too small\n");
    message->payload_length = 0;
    json_object_put(jobj);
    return -1;
  }

  strcpy(message->payload, payload);

  // decrements the reference count of the object and frees it if it reaches zero.
  json_object_put(jobj);
  return 0;
}
