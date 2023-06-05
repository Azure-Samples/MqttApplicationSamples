// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef MQTT_CLIENT_TEST_H
#define MQTT_CLIENT_TEST_H

#include "mqtt_setup.h"

typedef struct mqtt_client_test_state
{
  mqtt_client_connection_settings* connection_settings;
} mqtt_client_test_state;

int test_mqtt_client();

#endif // MQTT_CLIENT_TEST_H
