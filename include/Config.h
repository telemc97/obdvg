#ifndef OBDVG_CONFIG_H
#define OBDVG_CONFIG_H

#include "Types.h"

// Internal includes
#include "util/Logger.h"

namespace  Config {

    // CAN Related
    constexpr int32 CAN_TX_PIN = 6;
    constexpr int32 CAN_RX_PIN = 7;
    constexpr int32 CAN_BAUD = 115200;

    // MPU Related

    // DISPLAY Related

    // LOGGER Related
    constexpr LogLevel LOG_LEVEL = LogLevel::DEBUG;

    // FreeRTOS Task Settings
    constexpr uint32 CAN_TASK_STACK_SIZE = 2048;
    constexpr uint32 OBD_TASK_STACK_SIZE = 2048;
    constexpr uint32 MPU_TASK_STACK_SIZE = 4096;
    constexpr uint32 DISPLAY_TASK_STACK_SIZE = 4096;

    constexpr uint8 CAN_TASK_PRIORITY = 4;
    constexpr uint8 OBD_TASK_PRIORITY = 2;
    constexpr uint8 MPU_TASK_PRIORITY = 1;
    constexpr uint8 DISPLAY_TASK_PRIORITY = 3;

    // Queue Lengths
    constexpr uint8 CAN_TX_QUEUE_LENGTH = 8;
    constexpr uint8 CAN_RX_QUEUE_LENGTH = 8;
    constexpr uint8 MPU_DATA_QUEUE_LENGTH = 8;
    constexpr uint8 DISPLAY_QUEUE_LENGTH = 16;
    constexpr uint8 DISPLAY_MSG_SIZE = 16;


};


#endif //OBDVG_CONFIG_H