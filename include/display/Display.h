#ifndef OBDVG_DISPLAY_H
#define OBDVG_DISPLAY_H

#include "Types.h"

#include <cstring>

#include "hardware/i2c.h"

class Display {
public:
    /**
     * @brief Construct a new Display object.
     * @param i2c_instance The I2C instance to use (e.g., i2c0).
     * @param freqHz The I2C communication frequency in Hz.
     */
    Display(i2c_inst_t *i2c_instance, uint32 freqHz = 100000);

    /**
     * @brief Initializes the I2C peripheral and the display controller.
     * Must be called before using the display.
     */
    void init() const;

    /**
     * @brief Sets the brightness of the display.
     * @param level The brightness level, from 1 (dimmest) to 8 (brightest).
     */
    void setBrightness(uint8 level);

    /**
     * @brief Sets the 4-character text to be shown on the display.
     * The text is not displayed until update() is called.
     * @param str4 A null-terminated string of up to 4 characters.
     */
    void setText(const char *str4);

    /**
     * @brief Sends the current text buffer to the display.
     * This function should be called periodically to show the text set by setText().
     */
    void update() const;

    /**
     * @brief Checks if the display controller is connected and responsive.
     * @return True if the display is connected, false otherwise.
     */
    boolean isConnected() const;

private:
    /**
     * @brief Writes a single byte to a specific address on the I2C bus.
     * @param addr The I2C address of the target device/register.
     * @param data The byte to write.
     */
    void writeByte(uint8 addr, uint8 data) const;

    i2c_inst_t *i2c;
    uint32 freqHz;
    uint8 brightness;
    char text[5];
};

#endif //OBDVG_DISPLAY_H