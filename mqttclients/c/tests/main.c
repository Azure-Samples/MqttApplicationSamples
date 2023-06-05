// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "mqtt_client_test.h"

int main()
{
  int result = 0;

  result += test_mqtt_client();

  return result;
}