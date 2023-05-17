# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

set(CMAKE_CACHEFILE_DIR ${CMAKE_CURRENT_LIST_DIR}/build)

# MQTT Samples Executables
# telemetry_consumer
add_executable (telemetry_consumer
  ${MOSQUITTO_CLIENT_EXTENSIONS}
  ${CMAKE_CURRENT_LIST_DIR}/telemetry_consumer/main.c
)

# telemetry_producer
add_executable (telemetry_producer
  ${MOSQUITTO_CLIENT_EXTENSIONS}
  ${CMAKE_CURRENT_LIST_DIR}/telemetry_producer/main.c
)
