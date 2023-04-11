// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <mosquitto.h>

struct mosq_context{
	int messagesSent;
    int messagesReceived;
};

struct mosquitto *initMQTT(bool subscribe, bool publish, bool useTLS, struct mosq_context *context);

