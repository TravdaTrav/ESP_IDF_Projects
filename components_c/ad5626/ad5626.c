#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <string.h>

#include "ad5626.h"

#define AD5626_MAX_TICKS_WAIT 10

spi_device_handle_t ad5626_spi_dev;
gpio_num_t ad5626_ldac_pin;
gpio_num_t ad5626_clr_pin;

esp_err_t ad5626_spi_init(int8_t cs_pin, gpio_num_t ldac_pin, gpio_num_t clr_pin, spi_host_device_t spi_host)
{
    spi_device_interface_config_t ad5626_cfg = {
        .mode = 2,
        .clock_speed_hz = 1000000,
        .spics_io_num = cs_pin, // CS pin
        .flags = SPI_DEVICE_HALFDUPLEX,
        .queue_size = 1,
        .pre_cb = NULL,
        .post_cb = NULL,
    };

    ad5626_ldac_pin = ldac_pin;
    ad5626_clr_pin = clr_pin;

    esp_err_t ret = gpio_set_direction(ad5626_ldac_pin, GPIO_MODE_OUTPUT);
    if (ret)
    {
        return ret;
    }

    ret = gpio_set_level(ad5626_ldac_pin, 1);
    if (ret)
    {
        return ret;
    }

    if (ad5626_clr_pin >= 0)
    {
        ret = gpio_set_direction(ad5626_clr_pin, GPIO_MODE_OUTPUT);
        if (ret)
        {
            return ret;
        }

        ret = gpio_set_level(ad5626_clr_pin, 1);
        if (ret)
        {
            return ret;
        }
    }

    return spi_bus_add_device(spi_host, &ad5626_cfg, &ad5626_spi_dev);
}

esp_err_t ad5626_set_level(const uint16_t new_dac_level)
{
    uint8_t val_array[2];
    uint16_t shifted_new_level = new_dac_level << 4;
    memcpy(val_array, &shifted_new_level, 2);

    spi_transaction_t t = {
        .tx_buffer = val_array,
        .length = 12
    };

    esp_err_t ret = spi_device_polling_transmit(ad5626_spi_dev, &t);
    if (ret)
    {
        return ret;
    }

    ret = gpio_set_level(ad5626_ldac_pin, 0);
    if (ret)
    {
        return ret;
    }

    ret = gpio_set_level(ad5626_ldac_pin, 1);
    if (ret)
    {
        return ret;
    }

    return ESP_OK;
}

esp_err_t ad5626_clear_level_reg(void)
{
    if (ad5626_clr_pin < 0)
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = gpio_set_level(ad5626_clr_pin, 0);
    if (ret)
    {
        return ret;
    }

    ret = gpio_set_level(ad5626_clr_pin, 0);
    if (ret)
    {
        return ret;
    }

    return ESP_OK;
}