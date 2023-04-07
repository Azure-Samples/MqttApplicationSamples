# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

project (mqtt_samples LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(FetchContent)

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

add_subdirectory(${PRESET_PATH})