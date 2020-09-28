/**
 * @file tda74xx.c
 *
 * ESP-IDF driver for TDA7439/TDA7439DS/TDA7440
 *
 * Copyright (C) 2018 Ruslan V. Uss <unclerus@gmail.com>
 * MIT Licensed as described in the file LICENSE
 */

#include "tda74xx.h"
#include <esp_log.h>

#define I2C_FREQ_HZ 100000 // 100kHz

static const char *TAG = "TDA74xx";

#define REG_INPUT_SELECTOR 0x00
#define REG_INPUT_GAIN     0x01
#define REG_VOLUME         0x02
#define REG_BASS_GAIN      0x03
#define REG_MID_GAIN       0x04
#define REG_TREBLE_GAIN    0x05
#define REG_ATTEN_R        0x06
#define REG_ATTEN_L        0x07

#define MUTE_VALUE 0x38

#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)
#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

esp_err_t tda74xx_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    CHECK_ARG(dev);

    dev->port = port;
    dev->addr = TDA74XX_ADDR;
    dev->cfg.sda_io_num = sda_gpio;
    dev->cfg.scl_io_num = scl_gpio;
    dev->cfg.master.clk_speed = I2C_FREQ_HZ;

    CHECK(i2c_dev_create_mutex(dev));

    return ESP_OK;
}

esp_err_t tda74xx_free_desc(i2c_dev_t *dev)
{
    CHECK_ARG(dev);

    return i2c_dev_delete_mutex(dev);
}

esp_err_t tda74xx_set_input(i2c_dev_t *dev, uint8_t input)
{
    CHECK_ARG(dev);
    CHECK_ARG(input < 4);

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, REG_INPUT_SELECTOR, &input, 1));
    I2C_DEV_GIVE_MUTEX(dev);

    ESP_LOGD(TAG, "Input: %d", input);

    return ESP_OK;
}

esp_err_t tda74xx_set_input_gain(i2c_dev_t *dev, uint8_t gain_db)
{
    CHECK_ARG(dev);
    CHECK_ARG(gain_db < 31);

    uint8_t gain = gain_db / 2;

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, REG_INPUT_GAIN, &gain, 1));
    I2C_DEV_GIVE_MUTEX(dev);

    ESP_LOGD(TAG, "Input gain: %d dB", gain * 2);

    return ESP_OK;
}

esp_err_t tda74xx_set_volume(i2c_dev_t *dev, int8_t volume_db)
{
    CHECK_ARG(dev);
    CHECK_ARG((volume_db <= 0) && (volume_db >= -48));

    uint8_t volume = volume_db == -48 ? MUTE_VALUE : -volume_db;

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, REG_VOLUME, &volume, 1));
    I2C_DEV_GIVE_MUTEX(dev);

    ESP_LOGD(TAG, "Volume: %d dB", volume_db);

    return ESP_OK;
}

esp_err_t tda74xx_set_equalizer_gain(i2c_dev_t *dev, tda74xx_band_t band, int8_t gain_db)
{
    CHECK_ARG(dev);
    CHECK_ARG((gain_db >= -14) && (gain_db <= 14));

    uint8_t gain = (gain_db + 14) / 2;

    const char *name;
    uint8_t reg;
    switch(band)
    {
        case TDA74XX_BAND_BASS:
            name = "Bass";
            reg = REG_BASS_GAIN;
            break;
        case TDA74XX_BAND_MIDDLE:
            name = "Middle";
            reg = REG_MID_GAIN;
            break;
        default:
            name = "Treble";
            reg = REG_TREBLE_GAIN;
    }

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, reg, &gain, 1));
    I2C_DEV_GIVE_MUTEX(dev);

    ESP_LOGD(TAG, "%s gain: %d dB", name, gain * 2 - 14);

    return ESP_OK;
}

esp_err_t tda74xx_set_speaker_attenuation(i2c_dev_t *dev, tda74xx_channel_t channel, uint8_t atten_db)
{
    CHECK_ARG(dev);
    CHECK_ARG(atten_db <= 56);

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, channel == TDA74XX_CHANNEL_LEFT ? REG_ATTEN_L : REG_ATTEN_R, &atten_db, 1));
    I2C_DEV_GIVE_MUTEX(dev);

    ESP_LOGD(TAG, "Speaker attenuation (%c): %d dB", channel == TDA74XX_CHANNEL_LEFT ? 'L': 'R', atten_db);

    return ESP_OK;
}

