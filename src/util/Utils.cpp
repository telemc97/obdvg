#include "Types.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "util/Utils.h"
#include "util/Logger.h"


#include "pico/stdlib.h"

void Utils::floatToChars(float32 const value, char out[5]) {
    // Handle NaN or Inf
    if (!std::isfinite(value)) {
        strncpy(out, "ERR ", 5);
        return;
    }

    // Round to one decimal place for 4-char display
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", value);  // e.g. 12.3, -0.5, 3.1

    // If longer than 4, truncate (Display::setText will also pad/truncate)
    size_t len = strlen(buf);
    if (len > 4) len = 4;

    memcpy(out, buf, len);

    // Pad with spaces if shorter
    if (len < 4)
        memset(out + len, ' ', 4 - len);

    out[4] = '\0';
}

#include "util/Logger.h"

void Utils::scanI2cBus(i2c_inst_t *i2c, uint8_t sdaPin, uint8_t sclPin) {
    gpio_set_function(sdaPin, GPIO_FUNC_I2C);
    gpio_pull_up(sdaPin);
    gpio_set_function(sclPin, GPIO_FUNC_I2C);
    gpio_pull_up(sclPin);

    Logger::instance().log(LogLevel::INFO, "\nI2C Bus Scan");
    Logger::instance().log(LogLevel::INFO, "   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");

    char line[80];
    int offset = 0;

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            offset = snprintf(line, sizeof(line), "%02x ", addr);
        }

        uint8_t dummy = 0;
        int ret = i2c_write_blocking(i2c, addr, &dummy, 1, false);

        if (ret >= 0) {
            offset += snprintf(line + offset, sizeof(line) - offset, " %02x", addr);
        } else {
            offset += snprintf(line + offset, sizeof(line) - offset, " --");
        }
        if (addr % 16 == 15) {
            Logger::instance().log(LogLevel::INFO, line);
        }
    }
    gpio_deinit(sdaPin);
    gpio_disable_pulls(sdaPin);
    gpio_deinit(sclPin);
    gpio_disable_pulls(sclPin);

    Logger::instance().log(LogLevel::INFO, "Done.");

}
