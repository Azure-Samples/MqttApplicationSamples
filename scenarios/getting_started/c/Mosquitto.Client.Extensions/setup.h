/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <mosquitto.h>

struct mosq_context
{
    int messagesSent;
    int messagesReceived;
};

struct connection_settings
{
    char * hostname;
    int tcp_port;
    char * client_id;
    char * ca_file;
    char * ca_path;
    char * cert_file;
    char * key_file;
    char * key_file_password;
    int qos;
    int keep_alive_in_seconds;
    bool use_TLS;
    bool clean_session;
    int mqtt_version;
    char * username;
    char * password;
    char * sub_topic;
};

struct mosquitto * initMQTT( bool publish,
                             char * envFile,
                             struct mosq_context * context,
                             struct connection_settings * cs );