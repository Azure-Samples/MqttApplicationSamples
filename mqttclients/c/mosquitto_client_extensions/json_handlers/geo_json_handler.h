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
} mosquitto_payload;

// Reference generic geojson struct if another type is needed in the future
// typedef struct geojson_struct
// {
//     char* type;
//     void* data;
// } geojson_struct;

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
 * @param output The geojson_point to output to
 * @return int 0 on success, -1 on failure
 */
int mosquitto_payload_to_geojson_point(
    const struct mosquitto_message* message,
    geojson_point* output);

/**
 * @brief Converts a geojson_point to a mosquitto_payload
 *
 * @param geojson_point The geojson_point to convert
 * @param message The mosquitto_payload to output to
 * @return int 0 on success, -1 on failure
 */
int geojson_point_to_mosquitto_payload(
    const geojson_point geojson_point,
    mosquitto_payload* message);

/**
 * @brief Sets the coordinates of a geojson_point
 *
 * @param pt The geojson_point to set the coordinates of
 * @param x The x coordinate
 * @param y The y coordinate
 */
void geojson_point_set_coordinates(geojson_point* pt, double x, double y);

/**
 * @brief Initializes a geojson_point
 *
 * @return geojson_point The initialized geojson_point
 */
geojson_point geojson_point_init();

/**
 * @brief Initializes a mosquitto_payload
 *
 * @return mosquitto_payload The initialized mosquitto_payload
 */
mosquitto_payload mosquitto_payload_init();

/**
 * @brief Frees the memory of a mosquitto_payload
 *
 * @param payload The mosquitto_payload to free
 */
void mosquitto_payload_destroy(mosquitto_payload* payload);

#endif /* GEO_JSON_HANDLER_H */