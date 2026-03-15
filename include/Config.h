#ifndef OBDVG_CONFIG_H
#define OBDVG_CONFIG_H

#include "Types.h"

// Internal includes
#include "FreeRTOSConfig.h"
#include "obd/ObdPids.h"
#include "obd/ObdLogMessage.h"

namespace Config {

    // CAN UART Related (Waveshare TTL UART to CAN)
    constexpr int32 CAN_UART_BAUD = 115200;
    constexpr uint8 CAN_UART_TX_PIN = 4; // UART1 TX
    constexpr uint8 CAN_UART_RX_PIN = 5; // UART1 RX
    #define CAN_UART_INST uart1

    // SD Card SPI Related
    constexpr uint8 SD_SPI_SCK_PIN = 18;
    constexpr uint8 SD_SPI_MOSI_PIN = 19;
    constexpr uint8 SD_SPI_MISO_PIN = 16;
    constexpr uint8 SD_SPI_CS_PIN = 17;
    #define SD_SPI_INST spi0

    // Bluetooth Related
    const char* const BT_DEVICE_NAME = "OBDvg-ELM327";

    // LOGGER Related
    constexpr uint8 LOG_LEVEL = 3;

    // Simulation / Testing
    constexpr bool TEST_SIMULATOR_ENABLED = true;

    // FreeRTOS Task Settings

    // OBD Task (Handles CAN communication)
    constexpr uint8 OBD_TASK_PRIORITY = configMAX_PRIORITIES - 1;
    constexpr uint32 OBD_TASK_STACK_SIZE = 4096;
    constexpr uint32 OBD_TASK_DELAY_MS = 100;

    // ELM327 Bluetooth Task
    constexpr uint8 BT_TASK_PRIORITY = configMAX_PRIORITIES - 2;
    constexpr uint32 BT_TASK_STACK_SIZE = 4096;
    constexpr uint32 BT_TASK_DELAY_MS = 20;

    // SD Logging Task
    constexpr uint8 LOGGING_TASK_PRIORITY = configMAX_PRIORITIES - 3;
    constexpr uint32 LOGGING_TASK_STACK_SIZE = 4096;
    constexpr uint32 LOGGING_TASK_DELAY_MS = 500;

    // Queue Sizes
    constexpr uint8 CAN_RX_QUEUE_SIZE = 64;
    constexpr uint8 CAN_TX_QUEUE_SIZE = 64;
    constexpr uint8 LOG_QUEUE_SIZE = 128;

    // Logging Mapping: Link a PID to a field in ObdLogMessage
    struct LogMapping {
        ObdPid pid;
        float32 ObdLogMessage::*field;
    };

    static const LogMapping PID_LOG_CONFIG[] = {
        {ObdPid::ENGINE_RPM,          &ObdLogMessage::engine_speed},
        {ObdPid::VEHICLE_SPEED,       &ObdLogMessage::vehicle_speed},
        {ObdPid::ENGINE_COOLANT_TEMP, &ObdLogMessage::engine_coolant_temp},
        {ObdPid::ENGINE_OIL_TEMP,     &ObdLogMessage::engine_oil_temp},
        {ObdPid::THROTTLE_POS,        &ObdLogMessage::throttle_pos},
        {ObdPid::ENGINE_LOAD,         &ObdLogMessage::engine_load},
        {ObdPid::INTAKE_MAP,          &ObdLogMessage::intake_map},
        {ObdPid::INTAKE_AIR_TEMP,     &ObdLogMessage::intake_air_temp},
        {ObdPid::AMBIENT_AIR_TEMP,    &ObdLogMessage::ambient_air_temp},
        {ObdPid::CATALYST_TEMP_B1S1,  &ObdLogMessage::catalyst_temp},
        {ObdPid::SHORT_TERM_FUEL_TRIM_1, &ObdLogMessage::st_fuel_trim_1},
        {ObdPid::LONG_TERM_FUEL_TRIM_1,  &ObdLogMessage::lt_fuel_trim_1},
        {ObdPid::ENGINE_FUEL_RATE,    &ObdLogMessage::engine_fuel_rate}
    };

    static const uint8 PID_LOG_COUNT = sizeof(PID_LOG_CONFIG) / sizeof(PID_LOG_CONFIG[0]);

};

#endif //OBDVG_CONFIG_H
