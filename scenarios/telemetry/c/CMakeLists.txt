# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

set(CMAKE_CACHEFILE_DIR ${CMAKE_CURRENT_LIST_DIR}/build)
include_directories( ${CMAKE_CURRENT_LIST_DIR} ${MOSQUITTO_CLIENT_EXTENSIONS_DIR}/json_handlers)

find_package(json-c REQUIRED)

# External deps
link_libraries(
    json-c::json-c
)

# MQTT Samples Executables
# telemetry_consumer
add_executable (telemetry_consumer
  ${MOSQUITTO_CLIENT_EXTENSIONS}
  ${MOSQUITTO_CLIENT_EXTENSIONS_DIR}/json_handlers/geo_json_handler.c
  ${CMAKE_CURRENT_LIST_DIR}/telemetry_consumer/main.c
)

# telemetry_producer
add_executable (telemetry_producer
  ${MOSQUITTO_CLIENT_EXTENSIONS}
  ${MOSQUITTO_CLIENT_EXTENSIONS_DIR}/json_handlers/geo_json_handler.c
  ${CMAKE_CURRENT_LIST_DIR}/telemetry_producer/main.c
)
