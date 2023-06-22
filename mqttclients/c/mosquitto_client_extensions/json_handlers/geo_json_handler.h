/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#ifndef GEO_JSON_HANDLER_H
#define GEO_JSON_HANDLER_H

#include "mosquitto.h"
#include <json-c/json.h>

typedef struct mosquitto_payload
{
  char* payload;
  size_t payload_length;
  size_t max_payload_length;
} mosquitto_payload;

typedef struct geojson_coordinates
{
  double x, y;
} geojson_coordinates;

typedef struct geojson_point
{
  char* type;
  geojson_coordinates coordinates;
} geojson_point;

/**
 * @brief Converts a mosquitto_message to a geojson_point
 *
 * @param message The mosquitto_message to convert
 * @param output The geojson_point to output to. The type field must already be allocated. The
 * geojson_point_init() function will do this for you.
 * @return int 0 on success, -1 on failure
 */
int mosquitto_payload_to_geojson_point(
    const struct mosquitto_message* message,
    geojson_point* output);

/**
 * @brief Converts a geojson_point to a mosquitto_payload
 *
 * @param geojson_point The geojson_point to convert
 * @param message The mosquitto_payload to output to. Payload must already be allocated to a size of
 * max_payload_length (which must be set and will not be modified in this function).
 * mosquitto_payload_init() will do this for you.
 * @return int 0 on success, -1 on failure
 */
int geojson_point_to_mosquitto_payload(
    const geojson_point geojson_point,
    mosquitto_payload* message);

/**
 * @brief Sets the coordinates of a geojson_point
 *
 * @param pt Pointer to the geojson_point to set the coordinates of
 * @param x The x coordinate
 * @param y The y coordinate
 */
void geojson_point_set_coordinates(geojson_point* pt, double x, double y);

/**
 * @brief Initializes an empty geojson_point with type allocated to a length of "Point" and
 * coordinates set to (0, 0). The geojson_point must be freed with geojson_point_destroy().
 *
 * @return geojson_point The initialized geojson_point
 */
geojson_point geojson_point_init();

/**
 * @brief Frees the memory of a geojson_point's type field and sets the coordinates to 0.
 *
 * @param pt The geojson_point to free
 */
void geojson_point_destroy(geojson_point* pt);

/**
 * @brief Initializes a mosquitto_payload with payload_length set to 0 and payload allocated to
 * max_payload_length. The mosquitto_payload must be freed with mosquitto_payload_destroy().
 *
 * @param max_payload_length The maximum length of the payload - this is used to allocate memory for
 * the payload and ensure more memory is not written to the payload.
 * @return mosquitto_payload The initialized mosquitto_payload
 */
mosquitto_payload mosquitto_payload_init(int max_payload_length);

/**
 * @brief Frees the memory of a mosquitto_payload's payload and sets the payload_length and
 * max_payload_length to 0.
 *
 * @param payload The mosquitto_payload to free
 */
void mosquitto_payload_destroy(mosquitto_payload* payload);

#endif /* GEO_JSON_HANDLER_H */
