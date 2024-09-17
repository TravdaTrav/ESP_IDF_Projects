/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_task_wdt.h"
#include "esp_log.h"

#include "ads1120.hpp"
#include "ad5626.hpp"

#include "wifi_manager.h"
#include "mqtt_manager.h"

#define SPI_MOSI_PIN    10
#define SPI_MISO_PIN    9
#define SPI_SCLK_PIN    8

#define ADC_CS_PIN      5
#define ADC_DRDY_PIN    4

#define RTOS_QUEUE_SIZE                 10

esp_err_t start_adc(ADS1120& adc);

static const char* TAG = "main";

static TaskHandle_t read_adc_handle;

static TaskHandle_t send_mqtt_handle;

static SemaphoreHandle_t start_recording_task;

static QueueHandle_t data_queue = NULL;

static void send_mqtt_task(void* arg)
{
    ESP_LOGI(TAG, "Starting Send MQTT Task\n");

    wifi_start();

    wifi_connect();

    mqtt_start();

    esp_task_wdt_add(NULL);

    mqtt_message_t message;

    uint32_t count = 0;

    while (true)
    {
        if (uxQueueMessagesWaiting(data_queue) > 0)
        {
            xQueueReceive(data_queue, &message, 10);

            count++;

            if (!wifi_is_connected())
            {
                wifi_connect();
                mqtt_start();
            }

            if (mqtt_can_send_received())
            {
                mqtt_send_message((char*) &message, sizeof(mqtt_message_t));
            }

            if (count % 40 == 0)
            {
                ESP_LOGI(TAG, "Got 40 messages: %d\n", (int) xTaskGetTickCount());
                esp_task_wdt_reset();
            }
        }
        vTaskDelay(20);
    }
}

static void read_adc_task(void* arg)
{
    esp_task_wdt_add(NULL);

    ESP_LOGI(TAG, "Starting Read ADC Task\n");

    spi_bus_config_t spi_cfg;
    memset(&spi_cfg, 0, sizeof(spi_bus_config_t));

    spi_cfg.mosi_io_num = SPI_MOSI_PIN;
    spi_cfg.miso_io_num = SPI_MISO_PIN;
    spi_cfg.sclk_io_num = SPI_SCLK_PIN;
    spi_cfg.quadwp_io_num = -1;
    spi_cfg.quadhd_io_num = -1;

    esp_err_t err = spi_bus_initialize(SPI2_HOST, &spi_cfg, SPI_DMA_CH_AUTO);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to Initialize ADC\n");
    }

    ESP_LOGI(TAG, "Initialized SPI bus\n");

    ADS1120 ads1120;

    start_adc(ads1120);

    uint16_t data_val;

    uint32_t mes_buf_index = 0;

    mqtt_message_t mes_buf;
    mqtt_message_t discard_buf;

    mes_buf.start_mill = xTaskGetTickCount();
    esp_wifi_get_mac(WIFI_IF_STA, mes_buf.mac_address);

    uint32_t count = 0;

    while (true)
    {
        if (ads1120.isDataReady())
        {
            ads1120.readADC(&data_val);

            mes_buf.data[mes_buf_index] = data_val;

            mes_buf_index++;

            count++;

            if (count % 500 == 0)
            {
                esp_task_wdt_reset();
                vTaskDelay(1);
            }

            if (mes_buf_index == MQTT_MESSAGE_LENGTH)
            {
                if (uxQueueSpacesAvailable(data_queue) == 0){
                    xQueueReceive(data_queue, &discard_buf, (TickType_t) 10);
                }

                mes_buf.end_mill = xTaskGetTickCount();

                xQueueSend(data_queue, &mes_buf, (TickType_t) 10);

                mes_buf.start_mill = xTaskGetTickCount();
                mes_buf_index = 0;
            }
        }
    }
}

extern "C" void app_main(void)
{
    vTaskDelay(3000);

    start_recording_task = xSemaphoreCreateBinary();
    if (start_recording_task == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialize can_send semaphore\n");
    }

    data_queue = xQueueCreate(RTOS_QUEUE_SIZE, sizeof(mqtt_message_t));
    if (data_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialize data queue\n");
    }

    xTaskCreate(read_adc_task, "read_adc", 8192, NULL, 5,  &read_adc_handle);

    xTaskCreate(send_mqtt_task, "send_mqtt", 8192, NULL, 5, &send_mqtt_handle);
}

esp_err_t start_adc(ADS1120& adc)
{
    
    esp_err_t err = adc.init((gpio_num_t) ADC_CS_PIN, (gpio_num_t) ADC_DRDY_PIN, SPI2_HOST);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to Initialize ADC\n");
        return err;
    }

    ESP_LOGI(TAG, "Initialized ADC\n");

    err = adc.setGain(1);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to Set ADC Gain\n");
        return err;
    }

    ESP_LOGI(TAG, "Set ADC Gain\n");

    adc.setDataRate(0x06);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to Set ADC Data Rate\n");
        return err;
    }

    ESP_LOGI(TAG, "Set ADC Data Rate\n");

    adc.setOpMode(2);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to Set ADC Op Mode\n");
        return err;
    }

    ESP_LOGI(TAG, "Set ADC OP Mode\n");

    adc.setConversionMode(1);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to Set ADC Conversion Mode\n");
        return err;
    }

    ESP_LOGI(TAG, "Set ADC Conversion Mode\n");

    adc.setDRDYmode(1);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to Set ADC DRDY Mode\n");
        return err;
    }

    ESP_LOGI(TAG, "Set ADC DRDY Mode\n");

    adc.setMultiplexer(0x09);
    if (err)
    {
        ESP_LOGE(TAG, "Failed to Set ADC Multiplexer\n");
        return err;
    }

    ESP_LOGI(TAG, "Set ADC Multiplexer\n");

    return ESP_OK;
}