# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

set(CMAKE_CACHEFILE_DIR ${CMAKE_CURRENT_LIST_DIR}/build)
include_directories( ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/protobuf)

link_libraries(
    uuid
    protobuf-c
)

# MQTT Samples Executables
# command_server
add_executable (command_server
  ${MOSQUITTO_CLIENT_EXTENSIONS}
  ${CMAKE_CURRENT_LIST_DIR}/protobuf/google/protobuf/timestamp.pb-c.c
  ${CMAKE_CURRENT_LIST_DIR}/protobuf/unlock_command.pb-c.c
  ${CMAKE_CURRENT_LIST_DIR}/command_server/main.c
)

# command_client
add_executable (command_client
  ${MOSQUITTO_CLIENT_EXTENSIONS}
  ${CMAKE_CURRENT_LIST_DIR}/protobuf/google/protobuf/timestamp.pb-c.c
  ${CMAKE_CURRENT_LIST_DIR}/protobuf/unlock_command.pb-c.c
  ${CMAKE_CURRENT_LIST_DIR}/command_client/main.c
)
