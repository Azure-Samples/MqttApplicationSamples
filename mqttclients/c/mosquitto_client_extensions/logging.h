/* Copyright (c) Microsoft Corporation. All rights reserved. */
/* SPDX-License-Identifier: MIT */

#ifndef LOGGING_H
#define LOGGING_H

/*
Black:   \x1B[30m
Red:     \x1B[31m
Green:   \x1B[32m
Yellow:  \x1B[33m
Blue:    \x1B[34m
Magenta: \x1B[35m
Cyan:    \x1B[36m
White:   \x1B[37m
Dim:     \x1b[2m
Reset:   \x1B[0m
*/

#define MOSQUITTO_LOG_TAG "mosquitto"
#define MQTT_LOG_TAG "MQTT"
#define APP_LOG_TAG "App"
#define CLIENT_LOG_TAG "Client"
#define SERVER_LOG_TAG "Server"

#define LOG_INFO(log_tag, ...)                     \
  do                                               \
  {                                                \
    (void)printf("\x1B[34m[%s]\x1B[0m ", log_tag); \
    (void)printf(__VA_ARGS__);                     \
    (void)printf("\n");                            \
  } while (0)

#define LOG_ERROR(...)                                                       \
  do                                                                         \
  {                                                                          \
    (void)printf("\x1B[31m[ERROR]\x1B[0m " __VA_ARGS__);                     \
    (void)printf(" \x1b[2m[%s:%s:%d]\x1B[0m", __FILE__, __func__, __LINE__); \
    (void)printf("\n");                                                      \
  } while (0)

#define LOG_WARNING(...)                                   \
  do                                                       \
  {                                                        \
    (void)printf("\x1B[33m[WARNING]\x1B[0m " __VA_ARGS__); \
    (void)printf("\n");                                    \
  } while (0)

#endif /* LOGGING_H */
