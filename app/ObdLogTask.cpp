//
// Created by Tilemahos Mitroudas on 20/12/25.
//

#include "Types.h"
#include "can/UartCanBus.h"
#include "obd/ObdService.h"
#include "obd/ObdLogMessage.h"
#include "util/Utils.h"
#include "util/Logger.h"



float32 getPidValueFromCan(UartCanBus &can_bus_inst, ObdPid obd_pid, const CanFrame &tx_frame) {
    can_bus_inst.send(tx_frame);
    CanFrame rx_frame;
    float32 value = 0.0;
    if (!can_bus_inst.receive(rx_frame)) {
        Logger::instance().log(LogLevel::ERROR, "Timeout waiting for OBD response for PID: %d", ObdPid::ENGINE_RPM);
    } else {
        ObdService::pollResponse(rx_frame, obd_pid, value);
    }
    return value;
}

// OBD LOG TASK
void obdLogTask(void *pvParameters) {

    UartCanBus* can_bus = static_cast<UartCanBus*>(pvParameters);

    ObdLogMessage obd_log_msg{};

    boolean is_can_connected = true;

    for (;;) {
        boolean currentlyConnected = can_bus->isConnected();

        if (currentlyConnected != is_can_connected) {
            if (currentlyConnected) {
                Logger::instance().log(LogLevel::INFO, "CAN driver is connected");
            } else {
                Logger::instance().log(LogLevel::ERROR, "CAN driver is disconnected");
            }
            is_can_connected = currentlyConnected;
        }

        if (is_can_connected) {
            CanFrame tx_frame;
            const uint64 timestamp_start = Utils::getUnixTimestamp();

            ObdService::buildCanFrameForPID(ObdPid::THROTTLE_POS, tx_frame);
            obd_log_msg.throttle_pos = getPidValueFromCan(*can_bus, ObdPid::THROTTLE_POS, tx_frame);

            ObdService::buildCanFrameForPID(ObdPid::ENGINE_LOAD, tx_frame);
            obd_log_msg.engine_load = getPidValueFromCan(*can_bus, ObdPid::ENGINE_LOAD, tx_frame);

            ObdService::buildCanFrameForPID(ObdPid::ENGINE_COOLANT_TEMP, tx_frame);
            obd_log_msg.engine_coolant_temp = getPidValueFromCan(*can_bus, ObdPid::ENGINE_COOLANT_TEMP, tx_frame);

            ObdService::buildCanFrameForPID(ObdPid::ENGINE_OIL_TEMP, tx_frame);
            obd_log_msg.engine_oil_temp = getPidValueFromCan(*can_bus, ObdPid::ENGINE_OIL_TEMP, tx_frame);

            ObdService::buildCanFrameForPID(ObdPid::INTAKE_MAP, tx_frame);
            obd_log_msg.intake_map = getPidValueFromCan(*can_bus, ObdPid::INTAKE_MAP, tx_frame);

            ObdService::buildCanFrameForPID(ObdPid::VEHICLE_SPEED, tx_frame);
            obd_log_msg.vehicle_speed = getPidValueFromCan(*can_bus, ObdPid::VEHICLE_SPEED, tx_frame);

            ObdService::buildCanFrameForPID(ObdPid::INTAKE_AIR_TEMP, tx_frame);
            obd_log_msg.intake_air_temp = getPidValueFromCan(*can_bus, ObdPid::INTAKE_AIR_TEMP, tx_frame);

            ObdService::buildCanFrameForPID(ObdPid::ENGINE_RPM, tx_frame);
            obd_log_msg.engine_speed = getPidValueFromCan(*can_bus, ObdPid::ENGINE_RPM, tx_frame);

            const uint64 timestamp_end = Utils::getUnixTimestamp();
            const uint64 timestamp = timestamp_start + (timestamp_end - timestamp_start)/2;
            obd_log_msg.timestamp = timestamp;
        }

        vTaskDelay(pdMS_TO_TICKS(Config::MPU_V_TASK_DELAY));
    }
}
