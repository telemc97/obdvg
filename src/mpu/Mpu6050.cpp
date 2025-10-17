#include "Types.h"

#include <cstdio>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#include "Config.h"
#include "mpu/Mpu6050.h"
#include "util/Logger.h"

#define CALIBRATION_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

#define MPU6050_ADDR_BASE 0x69

// MPU6050 register map
static constexpr uint8 PWR_MGMT_1   = 0x6B;
static constexpr uint8 ACCEL_XOUT_H = 0x3B;
static constexpr uint8 TEMP_OUT_H   = 0x41;
static constexpr uint8 GYRO_XOUT_H  = 0x43;
static constexpr uint8 ACCEL_CONFIG = 0x1C;
static constexpr uint8 GYRO_CONFIG  = 0x1B;
static constexpr uint8 WHO_AM_I     = 0x75;

struct RawMpu6050Data {
    int16_t accel_x, accel_y, accel_z;
    int16_t temp;
    int16_t gyro_x, gyro_y, gyro_z;
};

Mpu6050::Mpu6050(i2c_inst_t* i2c_instance): i2c(i2c_instance) {}

boolean Mpu6050::isConnected() const {
    uint8 buf;
    if (!readRegisters(WHO_AM_I, &buf, 1)) {
        Logger::instance().log(LogLevel::DEBUG, "MPU6050: Failed to read WHO_AM_I register");
        return false;
    }
    if (buf != MPU6050_ADDR_BASE) {
        Logger::instance().log(LogLevel::DEBUG, "MPU6050 not found! WHO_AM_I=0x%02X", buf);
        return false;
    }
    return true;
}

boolean Mpu6050::init() {
    // Wake up the device
    if (!writeRegister(PWR_MGMT_1, 0x00)) {
        Logger::instance().log(LogLevel::DEBUG, "MPU6050: Failed to wake up device");
        return false;
    }
    sleep_ms(100);

    // Default ranges
    setAccelRange(AccelRange::AFS_2G);
    setGyroRange(GyroRange::FS_250DPS);

    return true;
}

boolean Mpu6050::readData(Mpu6050Data& data) {
    RawMpu6050Data raw_data;
    if (!readRegisters(ACCEL_XOUT_H, reinterpret_cast<uint8_t*>(&raw_data), sizeof(raw_data))) {
        Logger::instance().log(LogLevel::DEBUG, "MPU6050: Failed to read data");
        return false;
    }

    // The MPU6050 sends the data in big-endian format, so we need to swap the bytes.
    raw_data.accel_x = __builtin_bswap16(raw_data.accel_x);
    raw_data.accel_y = __builtin_bswap16(raw_data.accel_y);
    raw_data.accel_z = __builtin_bswap16(raw_data.accel_z);
    raw_data.temp = __builtin_bswap16(raw_data.temp);
    raw_data.gyro_x = __builtin_bswap16(raw_data.gyro_x);
    raw_data.gyro_y = __builtin_bswap16(raw_data.gyro_y);
    raw_data.gyro_z = __builtin_bswap16(raw_data.gyro_z);

    data.accel_x = accelRawToG(raw_data.accel_x) - accelOffsetX;
    data.accel_y = accelRawToG(raw_data.accel_y) - accelOffsetY;
    data.accel_z = accelRawToG(raw_data.accel_z) - accelOffsetZ;

    data.gyro_x = gyroRawToDps(raw_data.gyro_x) - gyroOffsetX;
    data.gyro_y = gyroRawToDps(raw_data.gyro_y) - gyroOffsetY;
    data.gyro_z = gyroRawToDps(raw_data.gyro_z) - gyroOffsetZ;

    data.temperature = tempRawToC(raw_data.temp);

    return true;
}

boolean Mpu6050::setAccelRange(AccelRange range) {
    if (!writeRegister(ACCEL_CONFIG, static_cast<uint8>(range) << 3)) return false;

    switch (range) {
        case AccelRange::AFS_2G:  accelScale = 16384.0f; break;
        case AccelRange::AFS_4G:  accelScale = 8192.0f;  break;
        case AccelRange::AFS_8G:  accelScale = 4096.0f;  break;
        case AccelRange::AFS_16G: accelScale = 2048.0f;  break;
    }
    return true;
}

boolean Mpu6050::setGyroRange(GyroRange range) {
    if (!writeRegister(GYRO_CONFIG, static_cast<uint8>(range) << 3)) return false;

    switch (range) {
        case GyroRange::FS_250DPS:  gyroScale = 131.0f;  break;
        case GyroRange::FS_500DPS:  gyroScale = 65.5f;   break;
        case GyroRange::FS_1000DPS: gyroScale = 32.8f;   break;
        case GyroRange::FS_2000DPS: gyroScale = 16.4f;   break;
    }
    return true;
}

void Mpu6050::setAccelOffsets(float32 x, float32 y, float32 z) {
    accelOffsetX = x;
    accelOffsetY = y;
    accelOffsetZ = z;
}

void Mpu6050::setGyroOffsets(float32 x, float32 y, float32 z) {
    gyroOffsetX = x;
    gyroOffsetY = y;
    gyroOffsetZ = z;
}

void Mpu6050::calibrate(uint16 samples) {
    float32 ax = 0, ay = 0, az = 0;
    float32 gx = 0, gy = 0, gz = 0;

    Mpu6050Data d{};
    for (uint16 i = 0; i < samples; i++) {
        if (readData(d)) {
            ax += d.accel_x;
            ay += d.accel_y;
            az += d.accel_z - 1.0f; // assume Z points up
            gx += d.gyro_x;
            gy += d.gyro_y;
            gz += d.gyro_z;
        }
        sleep_ms(2);
    }

    setAccelOffsets(ax / samples, ay / samples, az / samples);
    setGyroOffsets(gx / samples, gy / samples, gz / samples);
}

/**
 * @brief Calculates a checksum for the calibration data.
 * This is a simple XOR checksum.
 * @param d The calibration data.
 * @return The calculated checksum.
 */
static uint32 calcChecksum(const CalibrationData &d) {
    const auto *p = reinterpret_cast<const uint32*>(&d);
    uint32 sum = 0;
    for (size_t i = 0; i < (sizeof(CalibrationData)/4) - 1; i++) {
        sum ^= p[i];
    }
    return sum;
}

void Mpu6050::saveCalibration() {
    CalibrationData data {
        accelOffsetX, accelOffsetY, accelOffsetZ,
        gyroOffsetX, gyroOffsetY, gyroOffsetZ,
        0
    };
    data.checksum = calcChecksum(data);

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(CALIBRATION_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(CALIBRATION_FLASH_OFFSET,
                        reinterpret_cast<const uint8_t*>(&data),
                        sizeof(CalibrationData));
    restore_interrupts(ints);
}

boolean Mpu6050::loadCalibration() {
    const auto* data =
        reinterpret_cast<const CalibrationData*>(XIP_BASE + CALIBRATION_FLASH_OFFSET);

    if (data->checksum != calcChecksum(*data)) {
        Logger::instance().log(LogLevel::DEBUG, "MPU6050: Invalid calibration data checksum");
        return false; // invalid data
    }

    accelOffsetX = data->accelOffsetX;
    accelOffsetY = data->accelOffsetY;
    accelOffsetZ = data->accelOffsetZ;
    gyroOffsetX  = data->gyroOffsetX;
    gyroOffsetY  = data->gyroOffsetY;
    gyroOffsetZ  = data->gyroOffsetZ;

    return true;
}

boolean Mpu6050::writeRegister(uint8 reg, uint8 value) const {
    uint8 buf[2] = {reg, value};
    // Write the register address and the value in one go.
    int32 write_result = i2c_write_timeout_us(i2c, MPU6050_ADDR_BASE, buf, 2, false, Config::MPU_I2C_TIMEOUT);
    // Return true only if both bytes were written successfully.
    return write_result == 2;
}

boolean Mpu6050::readRegisters(uint8 reg, uint8* buffer, uint8 length) const {
    // First, write the starting register address without releasing the bus.
    int32 write_result = i2c_write_timeout_us(i2c, MPU6050_ADDR_BASE, &reg, 1, true, Config::MPU_I2C_TIMEOUT);
    if (write_result != 1) {
        return false; // Failed to write register address.
    }
    // Now, read the specified number of bytes.
    int32 read_result = i2c_read_timeout_us(i2c, MPU6050_ADDR_BASE, buffer, length, false, Config::MPU_I2C_TIMEOUT);
    // Return true only if the expected number of bytes were read.
    return read_result == length;
}

float32 Mpu6050::accelRawToG(const int16 raw) const {
    return static_cast<float32>(raw) / accelScale;
}

float32 Mpu6050::gyroRawToDps(const int16 raw) const {
    return static_cast<float32>(raw) / gyroScale;
}

float32 Mpu6050::tempRawToC(const int16 raw) {
    // From datasheet: Temp in °C = (raw / 340.0) + 36.53
    return (static_cast<float32>(raw) / 340.0f) + 36.53f;
}