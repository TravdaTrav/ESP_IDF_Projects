#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_system.h"

#include "wifi_manager.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define WIFI_SSID                       "PI_WIFI" 
#define WIFI_PASSWORD                   "12345"

static EventGroupHandle_t s_wifi_event_group;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_start(void)
{
    s_wifi_event_group = xEventGroupCreate();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

    esp_wifi_init(&wifi_init_config);

    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));

    memcpy(&wifi_config.sta.ssid, WIFI_SSID, sizeof(WIFI_SSID));
    memcpy(&wifi_config.sta.ssid, WIFI_PASSWORD, sizeof(WIFI_PASSWORD));

    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    esp_event_handler_instance_t instance_any_id;

    esp_event_handler_instance_t instance_got_ip;

    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_any_id);

    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_got_ip);

    esp_wifi_start();

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        printf("Connected to AP");
    }
}