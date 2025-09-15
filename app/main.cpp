#include "Types.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "can/CanFrame.h"
#include "can/UartCanBus.h"
#include "obd/ObdPids.h"
#include "obd/ObdService.h"
#include "util/Logger.h"

#include "pico/stdio.h"

#include <string>

// Queue handles
QueueHandle_t txQueue;
QueueHandle_t rxQueue;
QueueHandle_t logQueue;

// CAN bus instance using Waveshare TTL UART-CAN module
UartCanBus canBus(uart0, 0, 1, 115200); // TX=GP0, RX=GP1

// CAN TASK
void canTask(void *pvParameters) {
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

// OBD TASK
void obdTask(void *pvParameters) {
  CanFrame requestFrame;

  for (;;) {
    ObdService::buildCanFrameForPID(PID_ENGINE_RPM, requestFrame);
    // Send to CAN task
    xQueueSend(txQueue, &requestFrame, portMAX_DELAY);
    // Wait for response
    CanFrame rxFrame;
    while (xQueueReceive(rxQueue, &rxFrame, pdMS_TO_TICKS(100))) {
      float value;
      ObdService::pollResponse(rxFrame, PID_ENGINE_RPM, value);
      xQueueSend(logQueue, &value, portMAX_DELAY);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// LOGGER TASK
void loggerTask(void *pvParameters) {
  Logger logger;
  logger.init(LogLevel::DEBUG);
  String logMsg;
  for (;;) {
    if (xQueueReceive(logQueue, &logMsg, pdMS_TO_TICKS(50))) {
      logger.log(LogLevel::INFO, logMsg);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

int main() {
  stdio_init_all();

  // Create queues
  txQueue = xQueueCreate(8, sizeof(CanFrame));
  rxQueue = xQueueCreate(8, sizeof(CanFrame));
  logQueue = xQueueCreate(16, 64); // log messages

  if (!txQueue || !rxQueue || !logQueue) {
    for (;;) {
      __wfi();
    }
  }

  // Create tasks
  xTaskCreate(obdTask, "OBD", 2048, nullptr, 2, nullptr);
  xTaskCreate(loggerTask, "Logger", 1024, nullptr, 1, nullptr);

  // Start scheduler
  vTaskStartScheduler();

  // Should never reach here
  while (true) {
    __wfi();
  }
}