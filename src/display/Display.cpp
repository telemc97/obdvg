#include "Types.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include "pico/stdlib.h"
#include "Config.h"
#include "display/Display.h"

// Base addresses as per module spec
#define TM1650_ADDR_BASE 0x34
#define TM1650_CTRL_ADDR 0x24

// Character to 7-segment mapping (without dot)
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
        default:  return 0x00;
    }
}

Display::Display(i2c_inst_t *i2c_instance)
    : i2c(i2c_instance), brightness(8)
{
    memset(text, ' ', sizeof(text));
    text[4] = '\0';
    memset(dots, 0, sizeof(dots));
}

boolean Display::isConnected() const {
    uint8_t ctrl = 0x01;
    int ret = i2c_write_blocking(i2c, TM1650_CTRL_ADDR, &ctrl, 1, false);
    return (ret >= 0);
}

void Display::init() {
    uint8_t ctrl = 0x48 | ((brightness - 1) & 0x07);
    writeByte(TM1650_CTRL_ADDR, ctrl);
    sleep_ms(5);
    
    DisplayMsg msg{};
    msg.dot = -1;
    strncpy(msg.message, "HELO", 4);
    setText(msg);
    update();

}

void Display::setBrightness(uint8 level) {
    if (level < 1) level = 1;
    if (level > 8) level = 8;
    brightness = level;
    uint8_t ctrl = 0x48 | ((brightness - 1) & 0x07);
    writeByte(TM1650_CTRL_ADDR, ctrl);
    sleep_ms(2);
}

void Display::setText(const DisplayMsg &msg) {
    memcpy(text, msg.message, 4);
    text[4] = '\0';
    
    memset(dots, 0, 4);
    if (msg.dot >= 0 && msg.dot < 4) {
        dots[msg.dot] = 0x80;
    }
}

void Display::setDot(uint8 digitIndex, bool on) {
    if (digitIndex > 3) return;
    if (on) {
        dots[digitIndex] = 0x80;  // bit7 = dot / decimal point
    } else {
        dots[digitIndex] = 0x00;
    }
}

void Display::writeByte(uint8 addr, uint8 data) const {
    i2c_write_timeout_us(i2c, addr, &data, 1, false, Config::DISPLAY_I2C_TIMEOUT);
    sleep_us(200);
}

void Display::update() const {
    for (int i = 0; i < 4; i++) {
        uint8 seg = getCharSeg(text[i]) | dots[i];
        writeByte(TM1650_ADDR_BASE + i, seg);
    }
}

DisplayMsg Display::floatToChars(float32 const value) {
    DisplayMsg msg{};
    msg.dot = -1;
    memset(msg.message, ' ', 5); // Corrected size
    msg.message[4] = '\0';

    if (!std::isfinite(value)) {
        strncpy(msg.message, "ERR ", 4);
        return msg;
    }

    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", value);

    int msgPos = 0;
    for (int i = 0; buf[i] != '\0' && msgPos < 4; i++) {
        if (buf[i] == '.') {
            if (msgPos > 0) {
                msg.dot = msgPos - 1;
            }
        } else {
            msg.message[msgPos++] = buf[i];
        }
    }
    return msg;
}