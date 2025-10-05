//
// Created by Tilemahos Mitroudas on 4/10/25.
//

#include "Types.h"

#include "display/Display.h"

// Base addresses for digits
#define TM1650_ADDR_BASE 0x34
#define TM1650_CTRL_ADDR 0x24

// Converts a character to its 7-segment representation.
static uint8 getCharSeg(char c) {
    switch (c) {
        case '0': return 0x3F;
        case '1': return 0x06;
        case '2': return 0x5B;
        case '3': return 0x4F;
        case '4': return 0x66;
        case '5': return 0x6D;
        case '6': return 0x7D;
        case '7': return 0x07;
        case '8': return 0x7F;
        case '9': return 0x6F;
        case 'A': return 0x77;
        case 'B': return 0x7C;
        case 'C': return 0x39;
        case 'D': return 0x5E;
        case 'E': return 0x79;
        case 'F': return 0x71;
        case '-': return 0x40;
        case ' ': return 0x00;
        case '_': return 0x08;
        case 'H': return 0x76;
        case 'I': return 0x06;
        case 'L': return 0x38;
        case 'O': return 0x3F;
        case 'P': return 0x73;
        case 'S': return 0x6D;
        case 'U': return 0x3E;
        default: return 0x00; // Default to blank
    }
}

// Constructor
Display::Display(i2c_inst_t *i2c_instance, uint32 freqHz)
    : i2c(i2c_instance), freqHz(freqHz), brightness(8)
{
    memset(text, ' ', 4);
    text[4] = '\0';
}
void Display::init() const {
    i2c_init(i2c, freqHz);

    // Turn on display with default brightness
    writeByte(TM1650_CTRL_ADDR, 0x01 | brightness);
}

void Display::setBrightness(uint8 level) {
    if (level < 1) level = 1;
    if (level > 8) level = 8;
    brightness = level;
    writeByte(TM1650_CTRL_ADDR, 0x01 | brightness);
}

void Display::setText(const char *str4) {
    if (!str4) {
        memset(text, ' ', 4);
    } else {
        size_t len = strlen(str4);
        if (len > 4) len = 4;
        memcpy(text, str4, len);
        if (len < 4)
            memset(text + len, ' ', 4 - len);
    }
    text[4] = '\0';
}

void Display::writeByte(const uint8 addr, const uint8 data) const {
    i2c_write_blocking(i2c, addr, &data, 1, false);
}

void Display::update() const {
    for (int i = 0; i < 4; i++) {
        const char c = text[i];
        const uint8 seg = getCharSeg(c);
        writeByte(TM1650_ADDR_BASE + i, seg);
    }
}

boolean Display::isConnected() const {
    uint8_t data = 0;
    int ret = i2c_write_blocking(i2c, TM1650_CTRL_ADDR, &data, 1, false);
    return ret != PICO_ERROR_GENERIC;
}