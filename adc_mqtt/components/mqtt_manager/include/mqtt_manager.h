// Should use both types of header guards
#pragma once
#ifndef _MQTT_MANAGER_H_
#define _MQTT_MANAGER_H_

// Need to define functions as extern if compiling for C++
#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

#define MQTT_MESSAGE_LENGTH             50

typedef struct mqtt_message
{
    uint32_t milliseconds;
    uint16_t data[MQTT_MESSAGE_LENGTH];
} mqtt_message_t;

esp_err_t mqtt_start(void);

esp_err_t mqtt_send_message(const char* msg, const uint16_t msg_length);

bool mqtt_can_send_received(void);

// Need to define functions as extern if compiling for C++
#ifdef __cplusplus
}
#endif

#endif