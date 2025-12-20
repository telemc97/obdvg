//
// Created by Tilemahos Mitroudas on 20/12/25.
//

#include "Types.h"

#include "can/UartCanBus.h"
#include "obd/ObdService.h"
#include "util/Logger.h"
#include "display/Display.h"

void display(Display &display_inst, boolean &is_connected, const DisplayMsg &display_msg) {
    boolean currentlyConnected = display_inst.isConnected();

    if (currentlyConnected != is_connected) {
        if (currentlyConnected) {
            Logger::instance().log(LogLevel::INFO, "Display is connected");
            display_inst.init();
        } else {
            Logger::instance().log(LogLevel::ERROR, "Display is disconnected");
            return;
        }
        is_connected = currentlyConnected;
    }
    display_inst.setText(display_msg);
    display_inst.update();
}

// OBD DISPLAY TASK
void obdDisplayTask(void *pvParameters) {
    UartCanBus* can_bus = static_cast<UartCanBus*>(pvParameters);
    Display display_inst = Display(i2c1);
    boolean is_display_connected = true;
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
            ObdService::buildCanFrameForPID(ObdPid::ENGINE_RPM, tx_frame);
            can_bus->send(tx_frame);
            CanFrame rx_frame;
            if (!can_bus->receive(rx_frame)) {
                Logger::instance().log(LogLevel::ERROR, "Timeout waiting for OBD response for PID: %d", ObdPid::ENGINE_RPM);
            } else {
                float value;
                ObdService::pollResponse(rx_frame, ObdPid::ENGINE_RPM, value);

                DisplayMsg display_msg = Display::floatToChars(value);
                display(display_inst, is_display_connected, display_msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(Config::OBD_V_TASK_DELAY));
    }
}