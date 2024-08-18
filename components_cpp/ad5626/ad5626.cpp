#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <string.h>

#include "ad5626.hpp"

AD5626::AD5626()
{
}

esp_err_t AD5626::init(const gpio_num_t cs_pin, const gpio_num_t ldac_pin, const gpio_num_t clr_pin, const spi_host_device_t spi_host)
{
    this->ldac_pin = ldac_pin;
    this->clr_pin = clr_pin;

    spi_device_interface_config_t ad5626_cfg;

    ad5626_cfg.mode = 2;
    ad5626_cfg.clock_speed_hz = 1000000;
    ad5626_cfg.spics_io_num = cs_pin;
    ad5626_cfg.flags = SPI_DEVICE_HALFDUPLEX;
    ad5626_cfg.queue_size = 1;
    ad5626_cfg.pre_cb = NULL;
    ad5626_cfg.post_cb = NULL;

    esp_err_t ret = gpio_set_direction(this->ldac_pin, GPIO_MODE_OUTPUT);
    if (ret)
    {
        return ret;
    }

    ret = gpio_set_level(this->ldac_pin, 1);
    if (ret)
    {
        return ret;
    }

    if (this->clr_pin >= 0)
    {
        ret = gpio_set_direction(this->clr_pin, GPIO_MODE_OUTPUT);
        if (ret)
        {
            return ret;
        }

        ret = gpio_set_level(this->clr_pin, 1);
        if (ret)
        {
            return ret;
        }
    }

    return spi_bus_add_device(spi_host, &ad5626_cfg, &(this->spi_dev));
}

esp_err_t AD5626::set_level(const uint16_t new_dac_level)
{
    
    uint8_t val_array[2];
    uint16_t shifted_new_level = new_dac_level << 4;
    memcpy(val_array, &shifted_new_level, 2);

    spi_transaction_t t;
    t.tx_buffer = val_array;
    t.length = 12;

    esp_err_t ret = spi_device_polling_transmit(this->spi_dev, &t);
    if (ret)
    {
        return ret;
    }

    ret = gpio_set_level(this->ldac_pin, 0);
    if (ret)
    {
        return ret;
    }

    ret = gpio_set_level(this->ldac_pin, 1);
    if (ret)
    {
        return ret;
    }

    return ESP_OK;
}

esp_err_t AD5626::clear_level(void)
{
    if (this->clr_pin < 0)
    {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = gpio_set_level(this->clr_pin, 0);
    if (ret)
    {
        return ret;
    }

    ret = gpio_set_level(this->clr_pin, 0);
    if (ret)
    {
        return ret;
    }

    return ESP_OK; 
}