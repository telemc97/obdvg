#include "Types.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "util/Utils.h"
#include "util/Logger.h"


#include "pico/stdlib.h"

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

uint64 Utils::getUnixTimestamp() {
    return static_cast<uint64>(time(nullptr));
}

uint32 Utils::initI2C(i2c_inst_t* i2c_instance, uint32 freq, uint8 sdaPin, uint8 sclPin) {
    const uint32 ret = i2c_init(i2c_instance, freq);
    gpio_set_function(sdaPin, GPIO_FUNC_I2C);
    gpio_pull_up(sdaPin);
    gpio_set_function(sclPin, GPIO_FUNC_I2C);
    gpio_pull_up(sclPin);
    return ret;
}