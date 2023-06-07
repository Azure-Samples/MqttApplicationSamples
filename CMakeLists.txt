# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

option(LOG_ALL_MOSQUITTO "Print all mosquitto logs" OFF)
option(ENABLE_UNIT_TESTS "Build unit tests" OFF)

# make LOG_ALL_MOSQUITTO option enabled to be visible to code
if(LOG_ALL_MOSQUITTO)
  add_compile_definitions(LOG_ALL_MOSQUITTO)
endif()

project (mqtt_samples LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(FetchContent)

if(ENABLE_UNIT_TESTS)
  include(CTest)
endif()

# Mosquitto library setup
option( DOCUMENTATION OFF )
option( WITH_CJSON OFF )
FetchContent_Declare(
    MOSQUITTO
    GIT_REPOSITORY https://github.com/eclipse/mosquitto.git
    GIT_TAG        v2.0.15
    GIT_PROGRESS   TRUE
)

if(MOSQUITTO_PATH)
  message(INFO " MOSQUITTO_PATH specified: {MOSQUITTO_PATH}, skipping fetch for Mosquitto")
else()
  set(FETCHCONTENT_QUIET FALSE) # To see progress
  FetchContent_MakeAvailable(MOSQUITTO)

  set(MOSQUITTO_PATH ${mosquitto_SOURCE_DIR})
  message(INFO "MOSQUITTO_PATH set to ${MOSQUITTO_PATH}")
endif()

# External deps
link_libraries(
    mosquitto
)

# Helper functions for all samples
file(GLOB MOSQUITTO_CLIENT_EXTENSIONS ${CMAKE_CURRENT_LIST_DIR}/mqttclients/c/mosquitto_client_extensions/*.c)
include_directories( ${CMAKE_CURRENT_LIST_DIR}/mqttclients/c/mosquitto_client_extensions)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(${PRESET_PATH})
