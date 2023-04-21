# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

set(CMAKE_CACHEFILE_DIR ${CMAKE_CURRENT_LIST_DIR}/build)

# MQTT Samples Executables
# getting_started
add_executable (getting_started
  ${MOSQUITTO_CLIENT_EXTENSIONS}
  ${CMAKE_CURRENT_LIST_DIR}/main.c
)
