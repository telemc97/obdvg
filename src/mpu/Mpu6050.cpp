#include "mpu/Mpu6050.h"

#include "hardware/flash.h"
#include "hardware/sync.h"

#include <cstdio>

#define CALIBRATION_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - 4096) // last 4 KB

// MPU6050 register map
static constexpr uint8 PWR_MGMT_1   = 0x6B;
static constexpr uint8 ACCEL_XOUT_H = 0x3B;
static constexpr uint8 TEMP_OUT_H   = 0x41;
static constexpr uint8 GYRO_XOUT_H  = 0x43;
static constexpr uint8 ACCEL_CONFIG = 0x1C;
static constexpr uint8 GYRO_CONFIG  = 0x1B;
static constexpr uint8 WHO_AM_I     = 0x75;

Mpu6050::Mpu6050(i2c_inst_t* i2c_instance, const uint8 i2c_address)
    : i2c(i2c_instance), address(i2c_address) {}

// --------------------------------------------------
// Init
// --------------------------------------------------
boolean Mpu6050::begin() {
    // Check WHO_AM_I register (should return 0x68)
    uint8 buf;
    if (!readRegisters(WHO_AM_I, &buf, 1)) return false;
    if (buf != 0x68) {
        printf("MPU6050 not found! WHO_AM_I=0x%02X\n", buf);
        return false;
    }

    // Wake up the device
    if (!writeRegister(PWR_MGMT_1, 0x00)) return false;
    sleep_ms(100);

    // Default ranges
    setAccelRange(AccelRange::AFS_2G);
    setGyroRange(GyroRange::FS_250DPS);

    return true;
}

// --------------------------------------------------
// Read data
// --------------------------------------------------
boolean Mpu6050::readData(Mpu6050Data& data) const {
    uint8 buffer[14];
    if (!readRegisters(ACCEL_XOUT_H, buffer, 14)) {
        return false;
    }

    int16 raw_ax   = (buffer[0] << 8) | buffer[1];
    int16 raw_ay   = (buffer[2] << 8) | buffer[3];
    int16 raw_az   = (buffer[4] << 8) | buffer[5];
    int16 raw_temp = (buffer[6] << 8) | buffer[7];
    int16 raw_gx   = (buffer[8] << 8) | buffer[9];
    int16 raw_gy   = (buffer[10] << 8) | buffer[11];
    int16 raw_gz   = (buffer[12] << 8) | buffer[13];

    data.accel_x = accelRawToG(raw_ax) - accelOffsetX;
    data.accel_y = accelRawToG(raw_ay) - accelOffsetY;
    data.accel_z = accelRawToG(raw_az) - accelOffsetZ;

    data.gyro_x = gyroRawToDps(raw_gx) - gyroOffsetX;
    data.gyro_y = gyroRawToDps(raw_gy) - gyroOffsetY;
    data.gyro_z = gyroRawToDps(raw_gz) - gyroOffsetZ;

    data.temperature = tempRawToC(raw_temp);

    return true;
}

// --------------------------------------------------
// Range configuration
// --------------------------------------------------
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

// --------------------------------------------------
// Calibration
// --------------------------------------------------
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

static uint32 calcChecksum(const CalibrationData &d) {
    const auto *p = reinterpret_cast<const uint32*>(&d);
    uint32 sum = 0;
    for (size_t i = 0; i < (sizeof(CalibrationData)/4) - 1; i++) {
        sum ^= p[i];
    }
    return sum;
}

// --------------------------------------------------
// Persistent storage (stubbed: depends on your platform)
// --------------------------------------------------
void Mpu6050::saveCalibration() const {
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

// --------------------------------------------------
// Helpers
// --------------------------------------------------
boolean Mpu6050::writeRegister(const uint8 reg, const uint8 value) const {
    uint8 buf[2] = {reg, value};
    return i2c_write_blocking(i2c, address, buf, 2, false) == 2;
}

boolean Mpu6050::readRegisters(const uint8 reg, uint8* buffer, const uint8 length) const {
    if (i2c_write_blocking(i2c, address, &reg, 1, true) != 1) return false;
    return i2c_read_blocking(i2c, address, buffer, length, false) == length;
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
