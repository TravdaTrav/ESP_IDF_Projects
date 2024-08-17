#pragma once
#ifndef _AD5626_HPP_
#define _AD5626_HPP_

class AD5626 {
    public:
        AD5626();
        esp_err_t init(gpio_num_t cs_pin, gpio_num_t ldac_pin, gpio_num_t clr_pin, spi_host_device_t spi_host);
    private:
};

#endif