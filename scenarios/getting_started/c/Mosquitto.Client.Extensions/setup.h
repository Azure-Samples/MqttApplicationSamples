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

struct connection_settings {
    char *broker_address;
    int broker_port;
    char *client_id;
    char *ca_file;
    char *ca_path;
    char *cert_file;
    char *key_file;
    int qos;
    int keep_alive_in_seconds;
    bool use_TLS;
    int mqtt_version;
};

struct mosquitto *initMQTT(bool subscribe, bool publish, bool useTLS, struct mosq_context *context);

