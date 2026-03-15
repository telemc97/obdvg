#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "Config.h"
#include "can/UartCanBus.h"
#include "obd/ObdService.h"
#include "obd/ObdLogMessage.h"
#include "util/Logger.h"

extern QueueHandle_t obdDataQueue;
extern QueueHandle_t canTxQueue;
extern QueueHandle_t canRxQueue;

void obdTask(void* pvParameters) {
    (void)pvParameters;
    
    // Initialize UART-to-CAN bus
    static UartCanBus can_bus(CAN_UART_INST, Config::CAN_UART_BAUD);
    gpio_set_function(Config::CAN_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(Config::CAN_UART_RX_PIN, GPIO_FUNC_UART);
    
    Logger::instance().log("OBD Task: Initialized");

    // Wait for connection
    while (!can_bus.isConnected()) {
        Logger::instance().log("OBD Task: Waiting for CAN adapter...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    Logger::instance().log("OBD Task: CAN adapter connected!");

    ObdLogMessage currentData = {};
    CanFrame frame = {};
    static uint8 pollIdx = 0;

    while (true) {
        // 1. Check for outgoing requests from other modules (e.g. ELM327)
        while (xQueueReceive(canTxQueue, &frame, 0) == pdTRUE) {
            if (Config::TEST_SIMULATOR_ENABLED) {
                // SIMULATOR: Intercept request and generate immediate fake response
                CanFrame resp = {};
                resp.id = (frame.id == 0x7DF) ? 0x7E8 : (frame.id + 8);
                resp.dlc = 8;
                resp.data[0] = 0x04; // Length
                resp.data[1] = frame.data[1] + 0x40; // Response Mode
                resp.data[2] = frame.data[2]; // PID
                resp.data[3] = 0x0B; // Fake High Byte (approx 3000 RPM if PID 0C)
                resp.data[4] = 0xB8; // Fake Low Byte
                xQueueSend(canRxQueue, &resp, 0);
                Logger::instance().log("SIM: Handled Request for PID %02X", frame.data[2]);
            } else {
                can_bus.send(frame);
            }
        }

        // 2. Read incoming frames from the vehicle
        if (!Config::TEST_SIMULATOR_ENABLED && can_bus.receive(frame)) {
            
            // STREAM 1: Forward RAW frame to Bluetooth Task (ELM327 Emulation)
            xQueueSend(canRxQueue, &frame, 0);

            // STREAM 2: Decode and Forward to SD Logging Task
            float32 val;
            bool decoded = false;
            
            // Global OBD-II Response Validation: ID 0x7E8-0x7EF, Service 0x01
            if (frame.id >= 0x7E8 && frame.id <= 0x7EF && (frame.data[1] & 0x3F) == 0x01) {
                ObdPid rxPid = static_cast<ObdPid>(frame.data[2]);

                // Table-driven decoding using Config::PID_LOG_CONFIG
                for (uint8 i = 0; i < Config::PID_LOG_COUNT; i++) {
                    const auto& mapping = Config::PID_LOG_CONFIG[i];
                    if (rxPid == mapping.pid) {
                        if (ObdService::pollResponse(frame, rxPid, val)) {
                            currentData.*(mapping.field) = val; // Direct member update
                            decoded = true;
                        }
                        break;
                    }
                }
            }
            
            if (decoded) {
                currentData.timestamp = to_ms_since_boot(get_absolute_time());
                xQueueSend(obdDataQueue, &currentData, 0);
            }
        }

        // 3. Periodic Background Polling (Round-Robin from Config Table)
        static uint32 lastPoll = 0;
        uint32 now = to_ms_since_boot(get_absolute_time());
        if (now - lastPoll > Config::OBD_TASK_DELAY_MS) {
            lastPoll = now;
            
            CanFrame req;
            ObdService::buildCanFrameForPID(Config::PID_LOG_CONFIG[pollIdx].pid, req);
            can_bus.send(req);
            
            pollIdx = (pollIdx + 1) % Config::PID_LOG_COUNT;
        }

        vTaskDelay(pdMS_TO_TICKS(5)); 
    }
}
