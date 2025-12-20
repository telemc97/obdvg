#ifndef OBDVG_CONFIG_H
#define OBDVG_CONFIG_H

#include "Types.h"

// Internal includes
#include "FreeRTOSConfig.h"

namespace  Config {

    // i2c Bus Related
    constexpr uint32 I2C_FREQUENCY = 100 * 1000;
    constexpr uint8 I2C_SDA_PIN = 2;
    constexpr uint8 I2C_SCL_PIN = 3;

    // CAN Related
    constexpr int32 CAN_BAUD = 115200;

    // MPU Related
    constexpr uint32 MPU_I2C_TIMEOUT = 100000;

    // DISPLAY Related
    constexpr uint32 DISPLAY_I2C_TIMEOUT = 100000;

    // LOGGER Related
    constexpr uint8 LOG_LEVEL = 3;

    // FreeRTOS Task Settings

    // OBD DISPLAY TASK
    constexpr uint8 OBD_DISPLAY_TASK_PRIORITY = configMAX_PRIORITIES - 2;
    constexpr uint32 OBD_DISPLAY_TASK_STACK_SIZE = 2048;
    constexpr uint32 OBD_V_TASK_DELAY = 500;

    // OBD LOG TASK
    constexpr uint8 OBD_LOG_TASK_PRIORITY = configMAX_PRIORITIES - 1;
    constexpr uint32 OBD_LOG_TASK_STACK_SIZE = 4096;
    constexpr uint32 OBD_LOG_V_TASK_DELAY = 500;

    // MPU TASK
    constexpr uint8 MPU_TASK_PRIORITY = configMAX_PRIORITIES - 1;
    constexpr uint32 MPU_TASK_STACK_SIZE = 4096;
    constexpr uint32 MPU_V_TASK_DELAY = 500;

    // Queue Lengths

    constexpr uint8 SD_OBD_TX_QUEUE_LENGTH = 60;
    constexpr uint8 SD_MPU_TX_QUEUE_LENGTH = 60;


};


#endif //OBDVG_CONFIG_H