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
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "ads1120.hpp"
#include "ad5626.hpp"
#include "component_template.h"

extern "C" void app_main(void)
{
    spi_bus_config_t spi_cfg;
    spi_cfg.mosi_io_num = 10;
    spi_cfg.miso_io_num = -1;
    spi_cfg.sclk_io_num = 12;
    spi_cfg.quadwp_io_num = -1;
    spi_cfg.quadhd_io_num = -1;
    spi_cfg.max_transfer_sz = 32;

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &spi_cfg, SPI_DMA_CH_AUTO);

    ADS1120 ads1120;

    printf("Hello world!\n");

    comp_temp_func();

    for (int i = 10; i >= 0; i--)
    {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
