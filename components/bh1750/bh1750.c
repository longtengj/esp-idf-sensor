/**
 * ESP-IDF driver for BH1750 light sensor
 *
 * Ported from esp-open-rtos
 * Copyright (C) 2017 Andrej Krutak <dev@andree.sk>
 *               2018 Ruslan V. Uss <unclerus@gmail.com>
 * BSD Licensed as described in the file LICENSE
 *
 * ROHM Semiconductor bh1750fvi-e.pdf
 */
#include "bh1750.h"
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>


#define OPCODE_HIGH  0x0
#define OPCODE_HIGH2 0x1
#define OPCODE_LOW   0x3

#define OPCODE_CONT 0x10
#define OPCODE_OT   0x20

#define I2C_FREQ_HZ 400000

static const char *TAG = "BH1750";

#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)
#define CHECK_ARG(VAL) do { if (!VAL) return ESP_ERR_INVALID_ARG; } while (0)

esp_err_t bh1750_init_desc(i2c_dev_t *dev, uint8_t addr, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    CHECK_ARG(dev);

    if (addr != BH1750_ADDR_LO && addr != BH1750_ADDR_HI)
    {
        ESP_LOGE(TAG, "Invalid I2C address");
        return ESP_ERR_INVALID_ARG;
    }

    dev->port = port;
    dev->addr = addr;
    dev->cfg.sda_io_num = sda_gpio;
    dev->cfg.scl_io_num = scl_gpio;
    dev->cfg.master.clk_speed = I2C_FREQ_HZ;

    CHECK(i2c_dev_create_mutex(dev));

    return ESP_OK;
}

esp_err_t bh1750_free_desc(i2c_dev_t *dev)
{
    CHECK_ARG(dev);

    return i2c_dev_delete_mutex(dev);
}

esp_err_t bh1750_setup(i2c_dev_t *dev, bh1750_mode_t mode, bh1750_resolution_t resolution)
{
    CHECK_ARG(dev);

    uint8_t opcode = mode == BH1750_MODE_CONTINIOUS ? OPCODE_CONT : OPCODE_OT;
    switch (resolution)
    {
        case BH1750_RES_LOW:  opcode |= OPCODE_LOW;   break;
        case BH1750_RES_HIGH: opcode |= OPCODE_HIGH;  break;
        default:              opcode |= OPCODE_HIGH2; break;
    }

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_write(dev, NULL, 0, &opcode, 1));
    I2C_DEV_GIVE_MUTEX(dev);

    ESP_LOGD(TAG, "bh1750_setup(PORT = %d, ADDR = 0x%02x, VAL = 0x%02x)", dev->port, dev->addr, opcode);

    return ESP_OK;
}

esp_err_t bh1750_read(i2c_dev_t *dev, uint16_t *level)
{
    CHECK_ARG(dev);
    CHECK_ARG(level);

    uint8_t buf[2];

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_read(dev, NULL, 0, buf, 2));
    I2C_DEV_GIVE_MUTEX(dev);

    *level = buf[0] << 8 | buf[1];
    *level = (*level * 10) / 12; // convert to LUX

    return ESP_OK;
}
