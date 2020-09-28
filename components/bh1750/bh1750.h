/**
 * ESP-IDF driver for BH1750 light sensor
 *
 * Ported from esp-open-rtos
 * Copyright (C) 2017 Andrej Krutak <dev@andree.sk>
 * Copyright (C) 2018 Ruslan V. Uss <unclerus@gmail.com>
 * BSD Licensed as described in the file LICENSE
 *
 * ROHM Semiconductor bh1750fvi-e.pdf
 */
#ifndef __BH1750_H__
#define __BH1750_H__

#include <stdint.h>
#include <i2cdev.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Possible chip addresses
 */
#define BH1750_ADDR_LO 0x23 //!< ADDR pin floating/low
#define BH1750_ADDR_HI 0x5c //!< ADDR pin high

/**
 * Measurement mode
 */
typedef enum
{
    BH1750_MODE_ONE_TIME = 0, //!< One time measurement
    BH1750_MODE_CONTINIOUS    //!< Continious measurement
} bh1750_mode_t;

/**
 * Measurement resolution
 */
typedef enum
{
    BH1750_RES_LOW = 0,  //!< 4 lx resolution, measurement time is usually 16 ms
    BH1750_RES_HIGH,     //!< 1 lx resolution, measurement time is usually 120 ms
    BH1750_RES_HIGH2     //!< 0.5 lx resolution, measurement time is usually 120 ms
} bh1750_resolution_t;

/**
 * @brief Initialize device descriptior
 * @param[out] dev Pointer to device descriptor
 * @param[in] addr I2C address, BH1750_ADDR_LO or BH1750_ADDR_HI
 * @param[in] port I2C port number
 * @param[in] sda_pin GPIO pin number for SDA
 * @param[in] scl_pin GPIO pin number for SCL
 * @return `ESP_OK` on success
 */
esp_err_t bh1750_init_desc(i2c_dev_t *dev, uint8_t addr, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);

/**
 * @brief Free device descriptor
 * @param dev Pointer to device descriptor
 * @return `ESP_OK` on success
 */
esp_err_t bh1750_free_desc(i2c_dev_t *dev);

/**
 * @brief Setup device parameters
 * @param dev Pointer to device descriptor
 * @param mode Measurement mode
 * @param resolution Measurement resolution
 * @return `ESP_OK` on success
 */
esp_err_t bh1750_setup(i2c_dev_t *dev, bh1750_mode_t mode, bh1750_resolution_t resolution);


/**
 * @brief Read LUX value from the device.
 * @param dev Pointer to device descriptor
 * @param[out] level read value in lux units
 * @return `ESP_OK` on success
 */
esp_err_t bh1750_read(i2c_dev_t *dev, uint16_t *level);

#ifdef __cplusplus
}
#endif

#endif /* __BH1750_H__ */
