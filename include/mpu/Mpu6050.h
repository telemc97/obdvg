#ifndef OBDVG_MPU6050_H
#define OBDVG_MPU6050_H

#include "Types.h"

#include <cstdint>

#include "hardware/i2c.h"

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
    /**
     * @brief Construct a new Mpu6050 object.
     * @param i2c_instance The I2C instance to use (e.g., i2c0).
     * @param sdaPin The SDA Pin
     * @param sclPin The SCL Pin
     */
    explicit Mpu6050(i2c_inst_t* i2c_instance);

    /**
     * @brief Checks if the MPU6050 is connected and responsive.
     * Verifies the WHO_AM_I register.
     * @return True if the sensor is found, false otherwise.
     */
    boolean isConnected() const;

    /**
     * @brief Initializes the MPU6050 sensor.
     * Wakes the device and sets default ranges.
     * @return True if initialization was successful, false otherwise.
     */
    boolean init();

    /**
     * @brief Reads the latest sensor data (acceleration, gyroscope, temperature).
     * @param[out] data Reference to an Mpu6050Data struct to store the results.
     * @return True if data was read successfully, false otherwise.
     */
    boolean readData(Mpu6050Data&);

    /**
     * @brief Sets the full-scale range of the accelerometer.
     * @param range The desired accelerometer range.
     * @return True if the range was set successfully, false otherwise.
     */
    boolean setAccelRange(AccelRange range);

    /**
     * @brief Sets the full-scale range of the gyroscope.
     * @param range The desired gyroscope range.
     * @return True if the range was set successfully, false otherwise.
     */
    boolean setGyroRange(GyroRange range);

    // Calibration
    /**
     * @brief Manually sets the accelerometer calibration offsets.
     * @param x Offset for the X-axis.
     * @param y Offset for the Y-axis.
     * @param z Offset for the Z-axis.
     */
    void setAccelOffsets(float32 x, float32 y, float32 z);

    /**
     * @brief Manually sets the gyroscope calibration offsets.
     * @param x Offset for the X-axis.
     * @param y Offset for the Y-axis.
     * @param z Offset for the Z-axis.
     */
    void setGyroOffsets(float32 x, float32 y, float32 z);

    /**
     * @brief Calibrates the sensor by averaging multiple readings to find offsets.
     * @note This method assumes the sensor is stationary on a flat surface.
     * @param samples The number of samples to average for calibration.
     */
    void calibrate(uint16 samples = 1000);

    // Persistent storage
    /**
     * @brief Saves the current calibration offsets to the device's flash memory.
     */
    void saveCalibration();

    /**
     * @brief Loads calibration offsets from the device's flash memory.
     * @return True if calibration was loaded and checksum is valid, false otherwise.
     */
    boolean loadCalibration();

private:
    i2c_inst_t* i2c;

    float32 accelScale = 16384.0f;
    float32 gyroScale  = 131.0f;

    float32 accelOffsetX = 0, accelOffsetY = 0, accelOffsetZ = 0;
    float32 gyroOffsetX  = 0, gyroOffsetY  = 0, gyroOffsetZ  = 0;

    /**
     * @brief Writes a single byte to a specific register on the MPU6050.
     * @param reg The register to write to.
     * @param value The byte to write.
     * @return True on success, false on failure.
     */
    boolean writeRegister(uint8 reg, uint8 value) const;

    /**
     * @brief Reads a sequence of bytes from a starting register on the MPU6050.
     * @param reg The starting register to read from.
     * @param[out] buffer Pointer to the buffer to store the read data.
     * @param length The number of bytes to read.
     * @return True on success, false on failure.
     */
    boolean readRegisters(uint8 reg, uint8* buffer, uint8 length) const;

    /**
     * @brief Converts a raw accelerometer reading to G's.
     * @param raw The raw 16-bit sensor value.
     * @return The value in G's.
     */
    float32 accelRawToG(int16 raw) const;

    /**
     * @brief Converts a raw gyroscope reading to degrees per second.
     * @param raw The raw 16-bit sensor value.
     * @return The value in degrees per second.
     */
    float32 gyroRawToDps(int16 raw) const;

    /**
     * @brief Converts a raw temperature reading to degrees Celsius.
     * @param raw The raw 16-bit sensor value.
     * @return The value in degrees Celsius.
     */
    static float32 tempRawToC(int16 raw);
};

#endif //OBDVG_MPU6050_H