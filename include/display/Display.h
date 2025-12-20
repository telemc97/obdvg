#ifndef OBDVG_DISPLAY_H
#define OBDVG_DISPLAY_H

#include "Types.h"

#include <cstring>

#include "hardware/i2c.h"

struct DisplayMsg {
    int8 dot;
    char message[5];
};

class Display {
public:
    /**
     * @brief Construct a new Display object.
     * @param i2c_instance The I2C instance to use (e.g., i2c0).
     */
    Display(i2c_inst_t *i2c_instance);

    /**
     * @brief Checks if the display controller is connected and responsive.
     * @return True if the display is connected, false otherwise.
     */
    boolean isConnected() const;

    /**
     * @brief Initializes the I2C peripheral and the display controller.
     * Must be called before using the display.
     */
    void init();

    /**
     * @brief Sets the brightness of the display.
     * @param level The brightness level, from 1 (dimmest) to 8 (brightest).
     */
    void setBrightness(uint8 level);

    /**
     * @brief Sets the text and dot to be shown on the display.
     * The text is not displayed until update() is called.
     * @param msg A DisplayMsg struct containing the 4-char message and dot index.
     */
    void setText(const DisplayMsg &msg);

    /**
     * @brief Controls the dot next to a digit on the display.
     * @param digit The digit index (0-3) to control the dot for.
     * @param on True to turn the dot on, false to turn it off.
     */
    void setDot(uint8 digit, bool on);

    /**
     * @brief Sends the current text buffer to the display.
     * This function should be called periodically to show the text set by setText().
     */
    void update() const;

    static DisplayMsg floatToChars(float32 const value);

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
    uint8 dots[4];
};

#endif //OBDVG_DISPLAY_H