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

#include "ads1120.hpp"
#include "ad5626.hpp"

#include "wifi_manager.h"
#include "mqtt_manager.h"

#define SPI_MOSI_PIN    10
#define SPI_MISO_PIN    9
#define SPI_SCLK_PIN    8

#define ADC_CS_PIN      5
#define ADC_DRDY_PIN    4

#define RTOS_QUEUE_SIZE                 40

static SemaphoreHandle_t start_recording_task;

static QueueHandle_t data_queue;

static void send_mqtt_task(void* arg)
{
    wifi_start();

    mqtt_start();

    if (mqtt_can_send_received())
    {
        xSemaphoreGive(start_recording_task);
    }
    mqtt_message_t message;

    uint32_t count = 0;

    while (true)
    {
        if (uxQueueMessagesWaiting(data_queue) > 0)
        {
            xQueueReceive(data_queue, &message, 10);

            count++;

            mqtt_send_message((char*) &message, sizeof(mqtt_message_t));

            if (count % 40 == 0)
            {
                printf("Got 40 samples: %d\n", (int) xTaskGetTickCount());
            }
        }
    }
}

static void read_adc_task(void* arg)
{
    printf("Starting Read ADC Task\n");

    spi_bus_config_t spi_cfg;
    memset(&spi_cfg, 0, sizeof(spi_bus_config_t));

    spi_cfg.mosi_io_num = SPI_MOSI_PIN;
    spi_cfg.miso_io_num = SPI_MISO_PIN;
    spi_cfg.sclk_io_num = SPI_SCLK_PIN;
    spi_cfg.quadwp_io_num = -1;
    spi_cfg.quadhd_io_num = -1;

    spi_bus_initialize(SPI2_HOST, &spi_cfg, SPI_DMA_CH_AUTO);

    ADS1120 ads1120;

    ads1120.init((gpio_num_t) ADC_CS_PIN, (gpio_num_t) ADC_DRDY_PIN, SPI2_HOST);

    ads1120.setGain(1);

    ads1120.setDataRate(0x06);

    ads1120.setOpMode(2);

    ads1120.setConversionMode(1);

    ads1120.setDRDYmode(1);

    ads1120.setMultiplexer(0x09);

    printf("Initialized ADC\n");

    xSemaphoreTake(start_recording_task, portMAX_DELAY);

    uint16_t data_val;

    uint32_t mes_buf_index = 0;

    mqtt_message_t mes_buf;
    mes_buf.milliseconds = xTaskGetTickCount();

    while (true)
    {
        if (ads1120.isDataReady())
        {
            ads1120.readADC(&data_val);

            mes_buf.data[mes_buf_index] = data_val;

            mes_buf_index++;

            if (mes_buf_index == MQTT_MESSAGE_LENGTH)
            {
                if (uxQueueSpacesAvailable(data_queue) == 0){
                    mqtt_message_t discard_buf;
                    xQueueReceive(data_queue, &discard_buf, (TickType_t) 10);
                }

                xQueueSend(data_queue, &mes_buf, (TickType_t) 10);

                mes_buf.milliseconds = xTaskGetTickCount();
                mes_buf_index = 0;
            }
        }
    }
}

extern "C" void app_main(void)
{
    data_queue = xQueueCreate(RTOS_QUEUE_SIZE, sizeof(mqtt_message_t));

    xTaskCreatePinnedToCore(read_adc_task, "read_adc", 1024, NULL, 2, NULL, 0);

    xTaskCreatePinnedToCore(send_mqtt_task, "send_mqtt", 1024, NULL, 2, NULL, 0);
}