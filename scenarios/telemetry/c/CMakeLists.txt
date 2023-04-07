# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

set(CMAKE_CACHEFILE_DIR ${CMAKE_CURRENT_LIST_DIR}/build)

# MQTT Samples Executables
message(${CMAKE_CURRENT_LIST_DIR})
# Telemetry Consumer
add_executable (mosq_mqtt_telemetry_consumer
  ${CMAKE_CURRENT_LIST_DIR}/telemetry_consumer/main.c
)

# Telemetry Producer
add_executable (mosq_mqtt_telemetry_producer
  ${CMAKE_CURRENT_LIST_DIR}/telemetry_producer/main.c
)
