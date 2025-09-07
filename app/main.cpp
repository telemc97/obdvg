#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "can/UartCanBus.h"
#include "obd/ObdService.h"
#include "util/Logger.h"
#include "can/CanFrame.h"

// Queue handles
QueueHandle_t txQueue;
QueueHandle_t rxQueue;
QueueHandle_t logQueue;

// CAN bus instance using Waveshare TTL UART-CAN module
UartCanBus canBus(uart0, 0, 1, 115200); // TX=GP0, RX=GP1

extern "C" void canTask(void* pvParameters) {
    CanFrame frame;
    CanFrame rxFrame;

    for (;;) {
        // Send queued frames
        if (xQueueReceive(txQueue, &frame, pdMS_TO_TICKS(10)) == pdPASS) {
            canBus.send(frame);
        }

        // Poll for received frames
        if (canBus.receive(rxFrame)) {
            xQueueSend(rxQueue, &rxFrame, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

extern "C" void obdTask(void* pvParameters) {
    ObdService obd(canBus);
    CanFrame requestFrame;

    for (;;) {
        // Build OBD-II request frame
        requestFrame.id = 0x7DF;
        requestFrame.dlc = 8;
        memset(requestFrame.data.data(), 0, 8);
        requestFrame.data[0] = 0x02;
        requestFrame.data[1] = 0x01; // service 01
        requestFrame.data[2] = 0x0C; // Engine RPM PID

        // Send to CAN task
        xQueueSend(txQueue, &requestFrame, portMAX_DELAY);

        // Wait for response
        CanFrame rxFrame;
        while (xQueueReceive(rxQueue, &rxFrame, pdMS_TO_TICKS(100))) {
            float rpm;
            if (obd.pollResponse(0x0C, rpm)) {
                char logMsg[64];
                snprintf(logMsg, sizeof(logMsg), "RPM: %.1f", rpm);
                xQueueSend(logQueue, &logMsg, 0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

extern "C" void loggerTask(void* pvParameters) {
    char logMsg[64];
    for (;;) {
        if (xQueueReceive(logQueue, &logMsg, pdMS_TO_TICKS(50))) {
            Logger::info("%s", logMsg);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main() {
    stdio_init_all();

    Logger::info("Starting FreeRTOS OBD-II reader with CAN/OBD/Logger tasks");

    // Create queues
    txQueue = xQueueCreate(8, sizeof(CanFrame));
    rxQueue = xQueueCreate(8, sizeof(CanFrame));
    logQueue = xQueueCreate(16, 64); // log messages

    if (!txQueue || !rxQueue || !logQueue) {
        Logger::info("Failed to create queues");
        while (true) { __wfi(); }
    }

    // Create tasks
    xTaskCreate(canTask, "CAN", 1024, nullptr, 3, nullptr);
    xTaskCreate(obdTask, "OBD", 2048, nullptr, 2, nullptr);
    xTaskCreate(loggerTask, "Logger", 1024, nullptr, 1, nullptr);

    // Start scheduler
    vTaskStartScheduler();

    // Should never reach here
    Logger::info("Scheduler exited unexpectedly");
    while (true) { __wfi(); }
}