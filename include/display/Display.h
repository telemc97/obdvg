#ifndef OBDVG_DISPLAY_H
#define OBDVG_DISPLAY_H

#include "Types.h"

#include <cstring>

#include "hardware/i2c.h"

class Display {
public:
    Display(i2c_inst_t *i2c_instance, uint32 freqHz = 100000);

    void init() const;
    void setBrightness(uint8 level);
    void setText(const char *str4);
    void update() const;
    boolean isConnected() const;

private:
    void writeByte(uint8 addr, uint8 data) const;

    i2c_inst_t *i2c;
    uint32 freqHz;
    uint8 brightness;
    char text[5];
};

#endif //OBDVG_DISPLAY_H