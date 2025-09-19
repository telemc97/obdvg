#include "Types.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "can/CanFrame.h"
#include "can/UartCanBus.h"
#include "obd/ObdPids.h"
#include "obd/ObdService.h"
#include "mpu/Mpu6050.h"
#include "util/Logger.h"

#include "pico/stdio.h"

#include <string>

// Queue handles
QueueHandle_t txCanQueue;
QueueHandle_t rxCanQueue;
QueueHandle_t logQueue;
QueueHandle_t mpuDataQueue;

// CAN bus instance using Waveshare TTL UART-CAN module
UartCanBus canBus(uart0, 0, 1, 115200); // TX=GP0, RX=GP1

// CAN TASK
[[noreturn]] void canTask(void *pvParameters) {
  CanFrame frame;
  CanFrame rxFrame;

  for (;;) {
    // Send queued frames
    while (xQueueReceive(txCanQueue, &frame, pdMS_TO_TICKS(10))) {
      canBus.send(frame);
    }
    // Poll for received frames
    if (canBus.receive(rxFrame)) {
      xQueueSend(rxCanQueue, &rxFrame, 0);
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

// OBD TASK
[[noreturn]] void obdTask(void *pvParameters) {
  CanFrame requestFrame;

  for (;;) {
    ObdService::buildCanFrameForPID(PID_ENGINE_RPM, requestFrame);
    // Send to CAN task
    xQueueSend(txCanQueue, &requestFrame, portMAX_DELAY);
    // Wait for response
    CanFrame rxFrame;
    while (xQueueReceive(rxCanQueue, &rxFrame, pdMS_TO_TICKS(100))) {
      float value;
      ObdService::pollResponse(rxFrame, PID_ENGINE_RPM, value);
      xQueueSend(logQueue, &value, portMAX_DELAY);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

//MPU6050 TASK
[[noreturn]] void mpu6050Task(void *pvParameters) {
  Mpu6050 mpu6050(i2c0);
  mpu6050.begin();

  Mpu6050Data mpu_6050data{};

  for (;;) {
    mpu6050.readData(mpu_6050data);
    xQueueSend(mpuDataQueue, &mpu_6050data, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// LOGGER TASK
[[noreturn]] void loggerTask(void *pvParameters) {
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
  txCanQueue = xQueueCreate(8, sizeof(CanFrame));
  rxCanQueue = xQueueCreate(8, sizeof(CanFrame));
  mpuDataQueue = xQueueCreate(8, sizeof(Mpu6050Data));
  logQueue = xQueueCreate(16, 64); // log messages

  if (!txCanQueue || !rxCanQueue || !logQueue) {
    for (;;) {
      __wfi();
    }
  }

  // Create tasks
  xTaskCreate(canTask, "CAN", 2048, nullptr, 1, nullptr);
  xTaskCreate(obdTask, "OBD", 2048, nullptr, 2, nullptr);
  xTaskCreate(mpu6050Task, "MPU6050", 4096, nullptr, 3, nullptr);
  xTaskCreate(loggerTask, "Logger", 1024, nullptr, 4, nullptr);

  // Start scheduler
  vTaskStartScheduler();

  // Should never reach here
  while (true) {
    __wfi();
  }
}