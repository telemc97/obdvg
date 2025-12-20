//
// Created by Tilemahos Mitroudas on 20/12/25.
//

#include "util/Logger.h"
#include "mpu/Mpu6050.h"

void mpu6050Task(void *pvParameters) {
    Mpu6050 mpu6050(i2c1);

    Mpu6050Data mpu_6050data{};

    boolean isConnected = true;

    for (;;) {

        boolean currentlyConnected = mpu6050.isConnected();

        if (currentlyConnected != isConnected) {
            if (currentlyConnected) {
                Logger::instance().log(LogLevel::INFO, "MPU is connected");
                mpu6050.init();
            } else {
                Logger::instance().log(LogLevel::ERROR, "MPU is disconnected");
            }
            isConnected = currentlyConnected;
        }

        if (!isConnected) {
            vTaskDelay(pdMS_TO_TICKS(Config::MPU_V_TASK_DELAY));
            continue;
        }

        mpu6050.readData(mpu_6050data);

        vTaskDelay(pdMS_TO_TICKS(Config::MPU_V_TASK_DELAY));
    }
}