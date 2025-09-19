//
// Created by Tilemahos Mitroudas on 18/9/25.
//

#ifndef OBDVG_MPU6050_H
#define OBDVG_MPU6050_H

#pragma once

#include "Types.h"

#include <cstdint>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

enum class AccelRange {
    AFS_2G  = 0,
    AFS_4G  = 1,
    AFS_8G  = 2,
    AFS_16G = 3
};

enum class GyroRange {
    FS_250DPS  = 0,
    FS_500DPS  = 1,
    FS_1000DPS = 2,
    FS_2000DPS = 3
};

struct Mpu6050Data {
    float32 accel_x;
    float32 accel_y;
    float32 accel_z;
    float32 gyro_x;
    float32 gyro_y;
    float32 gyro_z;
    float32 temperature;
};

struct CalibrationData {
    float accelOffsetX, accelOffsetY, accelOffsetZ;
    float gyroOffsetX,  gyroOffsetY,  gyroOffsetZ;
    uint32_t checksum;
};

class Mpu6050 {
public:
    explicit Mpu6050(i2c_inst_t* i2c_instance, uint8 i2c_address = 0x68);

    boolean begin();
    boolean readData(Mpu6050Data&) const;

    boolean setAccelRange(AccelRange range);
    boolean setGyroRange(GyroRange range);

    // Calibration
    void setAccelOffsets(float32 x, float32 y, float32 z);
    void setGyroOffsets(float32 x, float32 y, float32 z);
    void calibrate(uint16 samples = 1000);

    // Persistent storage
    void saveCalibration() const;
    boolean loadCalibration();

private:
    i2c_inst_t* i2c;
    uint8 address;

    float32 accelScale = 16384.0f;
    float32 gyroScale  = 131.0f;

    float32 accelOffsetX = 0, accelOffsetY = 0, accelOffsetZ = 0;
    float32 gyroOffsetX  = 0, gyroOffsetY  = 0, gyroOffsetZ  = 0;

    boolean writeRegister(uint8 reg, uint8 value) const;
    boolean readRegisters(uint8 reg, uint8* buffer, uint8 length) const;

    float32 accelRawToG(int16 raw) const;
    float32 gyroRawToDps(int16 raw) const;
    static float32 tempRawToC(int16 raw);
};

#endif //OBDVG_MPU6050_H