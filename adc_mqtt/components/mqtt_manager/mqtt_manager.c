#include <stdio.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "esp_log.h"

#include "mqtt_manager.h"

static const char* TAG = "mqtt_manager";

#define MQTT_PUBLISH_TOPIC              "esp32/publish"
#define MQTT_SUBSCRIBE_TOPIC            "esp32/can_send"
#define MQTT_PORT                       1883
#define MQTT_SEND_MESSAGE_THRESHHOLD    RTOS_QUEUE_SIZE / 20

static esp_mqtt_client_handle_t mqtt_handle;

static bool can_send_received;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t) event_id) 
    {
    case MQTT_EVENT_CONNECTED:
        esp_mqtt_client_subscribe(client, MQTT_SUBSCRIBE_TOPIC, 2);
        break;
    case MQTT_EVENT_DISCONNECTED:
        esp_mqtt_client_reconnect(mqtt_handle);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        break;
    case MQTT_EVENT_PUBLISHED:
        break;
    case MQTT_EVENT_DATA:
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if (memcmp("SEND", event->data, sizeof("SEND")) == 0)
        {
            can_send_received = true;
        }
        break;
    case MQTT_EVENT_ERROR:
        break;
    default:
        break;
    }
}

esp_err_t mqtt_start(void)
{
    esp_mqtt_client_config_t mqtt_config;
    memset(&mqtt_config, 0, sizeof(esp_mqtt_client_config_t));

    // TODO: find MQTT Broker hostname/URI
    mqtt_config.broker.address.uri = "mqtt://10.42.0.1";

    mqtt_handle = esp_mqtt_client_init(&mqtt_config);

    esp_err_t err = esp_mqtt_client_register_event(mqtt_handle, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    if (err != ESP_OK)
    {
        return err;
    }

    return esp_mqtt_client_start(mqtt_handle);
}

esp_err_t mqtt_send_message(const char* msg, const uint16_t msg_length)
{
    return esp_mqtt_client_publish(mqtt_handle, MQTT_PUBLISH_TOPIC, msg, msg_length, 2, 0);
}

bool mqtt_can_send_received(void)
{
    return can_send_received;
}
