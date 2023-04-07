# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

project (mqtt_samples LANGUAGES C)

set(CMAKE_C_STANDARD 99)

find_package(mosquitto CONFIG)
if(NOT mosquitto_FOUND)
  find_package(mosquitto REQUIRED)
endif()

find_package(OpenSSL CONFIG)
if(NOT OpenSSL_FOUND)
  find_package(OpenSSL REQUIRED)
endif()

include_directories(${MOSQUITTO_INCLUDE_DIRS})

# External deps
link_libraries(
    ${MOSQUITTO_LIBRARIES}
    ${PTHREAD_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
)
message(${PRESET_PATH})

add_subdirectory(${PRESET_PATH})