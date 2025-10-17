#ifndef OBDVG_UTILS_H
#define OBDVG_UTILS_H

#include "Types.h"
#include "hardware/i2c.h"

/**
 * @brief A collection of utility functions.
 */
class Utils {

public:
    /**
     * @brief Converts a float to a 4-character string for the display.
     *
     * The output is formatted to one decimal place. If the resulting string is
     * longer than 4 characters, it is truncated. If it's shorter, it's padded
     * with spaces. Handles NaN and Inf values by returning "ERR ".
     *
     * @param value The float value to convert.
     * @param out A character array of at least 5 bytes to store the result.
     */
    static void floatToChars(float32 const value, char out[5]);

    /**
     * @brief Scans the I2C bus for connected devices.
     *
     * This function initializes the specified I2C peripheral and pins,
     * then scans for devices on the bus. The results are printed to the
     * standard output (serial).
     *
     * @param i2c The I2C instance to use (e.g., i2c0).
     * @param sdaPin The GPIO pin for SDA.
     * @param sclPin The GPIO pin for SCL.
     */
    static void scanI2cBus(i2c_inst_t *i2c, uint8 sdaPin, uint8 sclPin);

};

#endif //OBDVG_UTILS_H