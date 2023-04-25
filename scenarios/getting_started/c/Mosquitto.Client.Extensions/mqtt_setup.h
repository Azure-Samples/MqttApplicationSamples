/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#ifndef MQTT_SETUP_H
#define MQTT_SETUP_H

typedef struct mqtt_client_connection_settings
{
    int keep_alive_in_seconds;
    int mqtt_version;
    int qos;
    int tcp_port;
    bool clean_session;
    bool use_TLS;
    char * ca_file;
    char * ca_path;
    char * cert_file;
    char * client_id;
    char * hostname;
    char * key_file;
    char * key_file_password;
    char * password;
    char * sub_topic;
    char * username;
} mqtt_client_connection_settings;

struct mosquitto * mqtt_client_init( bool publish,
                             char * env_file,
                             mqtt_client_connection_settings * connection_settings );

#endif /* MQTT_SETUP_H */