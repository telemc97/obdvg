#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"

#include "Types.h"
#include <string>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Config.h"
#include "can/CanFrame.h"
#include "can/UartCanBus.h"
#include "obd/ObdService.h"
#include "obd/ObdLogMessage.h"
#include "util/Logger.h"
#include "util/Utils.h"

#include "ObdTask.cpp"
#include "BluetoothTask.cpp"
#include "SdLoggingTask.cpp"

// Global Queues
QueueHandle_t obdDataQueue = nullptr;
QueueHandle_t canTxQueue = nullptr;
QueueHandle_t canRxQueue = nullptr;

int main() {
    // 1. Initialize hardware
    stdio_init_all();
    
    // Give some time for USB serial to connect
    sleep_ms(2000);
    
    Logger::instance().log("OBDvg Starting...");

    // 2. Create Queues
    obdDataQueue = xQueueCreate(Config::LOG_QUEUE_SIZE, sizeof(ObdLogMessage));
    canTxQueue = xQueueCreate(Config::CAN_TX_QUEUE_SIZE, sizeof(CanFrame));
    canRxQueue = xQueueCreate(Config::CAN_RX_QUEUE_SIZE, sizeof(CanFrame));

    if (obdDataQueue == nullptr || canTxQueue == nullptr || canRxQueue == nullptr) {
        Logger::instance().log("Failed to create queues!");
        return -1;
    }

    // 3. Create Tasks
    xTaskCreate(obdTask, "OBD_TASK", Config::OBD_TASK_STACK_SIZE, nullptr, Config::OBD_TASK_PRIORITY, nullptr);
    xTaskCreate(bluetoothTask, "BT_TASK", Config::BT_TASK_STACK_SIZE, nullptr, Config::BT_TASK_PRIORITY, nullptr);
    xTaskCreate(sdLoggingTask, "SD_LOG_TASK", Config::LOGGING_TASK_STACK_SIZE, nullptr, Config::LOGGING_TASK_PRIORITY, nullptr);

    // 4. Start Scheduler
    vTaskStartScheduler();

    while (true) {
        // Should never reach here
    }
    return 0;
}


