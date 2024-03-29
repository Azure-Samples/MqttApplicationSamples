# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (mqtt_client_test LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(CTest)
enable_testing()

find_package(json-c CONFIG)

add_library(mqtt_client_test_lib
    ${CMAKE_CURRENT_LIST_DIR}/../mosquitto_client_extensions/mqtt_callbacks.c
    ${CMAKE_CURRENT_LIST_DIR}/../mosquitto_client_extensions/mqtt_setup.c
    ${CMAKE_CURRENT_LIST_DIR}/../mosquitto_client_extensions/json_handlers/geo_json_handler.c
)

target_include_directories(mqtt_client_test_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../mosquitto_client_extensions
    ${CMAKE_CURRENT_LIST_DIR}/../mosquitto_client_extensions/json_handlers
)

# deps
link_libraries(
    mqtt_client_test_lib
    cmocka
    mosquitto
    json-c
)

add_executable(mqtt_extensions_test main.c mqtt_client_test.c json_handler_test.c)

add_test(NAME mqtt_extensions_test COMMAND mqtt_extensions_test)
