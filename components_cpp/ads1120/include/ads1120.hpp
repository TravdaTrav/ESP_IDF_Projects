#pragma once
#ifndef _ADS1120_HPP_
#define _ADS1120_HPP_

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>

#define ADS1120_SPI_MASTER_DUMMY   0xFF
// Commands for the ADC
#define ADS1120_CMD_RESET 0x07
#define ADS1120_CMD_START_SYNC 0x08
#define ADS1120_CMD_PWRDWN 0x03
#define ADS1120_CMD_RDATA 0x1f
#define ADS1120_CMD_RREG 0x20
#define ADS1120_CMD_WREG 0x40

// Configuration registers
#define ADS1120_CONFIG_REG0_ADDRESS 0x00
#define ADS1120_CONFIG_REG1_ADDRESS 0x01
#define ADS1120_CONFIG_REG2_ADDRESS 0x02
#define ADS1120_CONFIG_REG3_ADDRESS 0x03

// Register masks for setings
// Register 0
#define ADS1120_REG_MASK_MUX 0xF0
#define ADS1120_REG_MASK_GAIN 0x0E
#define ADS1120_REG_MASK_PGA_BYPASS 0x01

// Register 1
#define ADS1120_REG_MASK_DATARATE 0xE0
#define ADS1120_REG_MASK_OP_MODE 0x18
#define ADS1120_REG_MASK_CONV_MODE 0x04
#define ADS1120_REG_MASK_TEMP_MODE 0x02
#define ADS1120_REG_MASK_BURNOUT_SOURCES 0x01

// Register 2
#define ADS1120_REG_MASK_VOLTAGE_REF 0xC0
#define ADS1120_REG_MASK_FIR_CONF 0x30
#define ADS1120_REG_MASK_PWR_SWITCH 0x08
#define ADS1120_REG_MASK_IDAC_CURRENT 0x07

// Register 3
#define ADS1120_REG_MASK_IDAC1_ROUTING 0xE0
#define ADS1120_REG_MASK_IDAC2_ROUTING 0x1C
#define ADS1120_REG_MASK_DRDY_MODE 0x02
#define ADS1120_REG_MASK_RESERVED 0x01

class ADS1120 {
  public:
    ADS1120();

    esp_err_t writeRegister(uint8_t address, uint8_t value);
    esp_err_t readRegister(uint8_t address, uint8_t* data);
    esp_err_t init(gpio_num_t cs_pin, gpio_num_t drdy_pin, spi_host_device_t spi_host);
    bool isDataReady(void);
    esp_err_t readADC(uint16_t* data);
    esp_err_t sendCommand(uint8_t command);
    esp_err_t reset(void);
    esp_err_t startSync(void);
    esp_err_t powerDown(void);
    esp_err_t rdata(void);
    esp_err_t writeRegisterMasked(uint8_t value, uint8_t mask, uint8_t address);
    esp_err_t setMultiplexer(uint8_t value);
    esp_err_t setGain(uint8_t gain);
    esp_err_t setPGAbypass(bool value);
    esp_err_t setDataRate(uint8_t value);
    esp_err_t setOpMode(uint8_t value);
    esp_err_t setConversionMode(uint8_t value);
    esp_err_t setTemperatureMode(uint8_t value);
    esp_err_t setBurnoutCurrentSources(bool value);
    esp_err_t setVoltageRef(uint8_t value);
    esp_err_t setFIR(uint8_t value);
    esp_err_t setPowerSwitch(uint8_t value);
    esp_err_t setIDACcurrent(uint8_t value);
    esp_err_t setIDAC1routing(uint8_t value);
    esp_err_t setIDAC2routing(uint8_t value);
    esp_err_t setDRDYmode(uint8_t value);
  private:
    gpio_num_t ADS1120_CS_PIN;
    gpio_num_t ADS1120_DRDY_PIN;
	  spi_device_handle_t spi_dev;
};

#endif