#include "Types.h"

#include <string>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "pico/stdio.h"
#include "queue.h"
#include "task.h"

#include "Config.h"
#include "can/CanFrame.h"
#include "can/UartCanBus.h"
#include "display/Display.h"
#include "mpu/Mpu6050.h"
#include "obd/ObdPids.h"
#include "obd/ObdService.h"
#include "util/Logger.h"
#include "util/Utils.h"

// Queue handles
QueueHandle_t txCanQueue;
QueueHandle_t rxCanQueue;
QueueHandle_t mpuDataQueue;
QueueHandle_t displayQueue;


// CAN TASK
[[noreturn]] void canTask(void *pvParameters) {
  UartCanBus canBus(uart0, Config::CAN_BAUD);

  if (!canBus.isConnected()) {
    Logger::instance().log(LogLevel::ERROR, "Cannot connect to the CAN driver");
  }

  CanFrame txFrame;
  CanFrame rxFrame;

  for (;;) {
    if (xQueueReceive(txCanQueue, &txFrame, pdMS_TO_TICKS(10))) {
      boolean rxStatus = canBus.send(txFrame);
      if (!rxStatus) {
        Logger::instance().log(LogLevel::ERROR, "Can send failed");
      }
    }
    boolean txStatus = canBus.receive(rxFrame);
    if (txStatus) {
      if (xQueueSend(rxCanQueue, &rxFrame, 0) != pdPASS) {
        Logger::instance().log(LogLevel::ERROR, "Failed to send received CAN frame to queue");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

// OBD TASK
[[noreturn]] void obdTask(void *pvParameters) {
  for (;;) {

    CanFrame requestFrame;
    ObdService::buildCanFrameForPID(PID_ENGINE_RPM, requestFrame);
    if (xQueueSend(txCanQueue, &requestFrame, portMAX_DELAY) != pdPASS) {
      Logger::instance().log(LogLevel::ERROR, "Failed to send OBD request to CAN task");
    }

    CanFrame rxFrame;
    if (xQueueReceive(rxCanQueue, &rxFrame, pdMS_TO_TICKS(100))) {
      float value;
      ObdService::pollResponse(rxFrame, PID_ENGINE_RPM, value);
      char displayMsg[Config::DISPLAY_MSG_SIZE];
      Utils::floatToChars(value, displayMsg);
      if (xQueueSend(displayQueue, &displayMsg, portMAX_DELAY) != pdPASS) {
        Logger::instance().log(LogLevel::ERROR, "Failed to send display message");
      }
    } else {
      Logger::instance().log(LogLevel::ERROR, "Timeout waiting for OBD response");
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// MPU6050 TASK
[[noreturn]] void mpu6050Task(void *pvParameters) {
  Mpu6050 mpu6050(i2c1);

  if (!mpu6050.isConnected()) {
    Logger::instance().log(LogLevel::ERROR, "Cannot connect to the IMU");
  }

  mpu6050.begin();

  Mpu6050Data mpu_6050data{};

  for (;;) {
    mpu6050.readData(mpu_6050data);
    if (xQueueSend(mpuDataQueue, &mpu_6050data, 0) != pdPASS) {
        Logger::instance().log(LogLevel::ERROR, "Failed to send MPU data to queue");
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// DISPLAY TASK
[[noreturn]] void displayTask(void *pvParameters) {
  Display display = Display(i2c1);
  display.init();

  if (!display.isConnected()) {
    Logger::instance().log(LogLevel::ERROR, "Cannot connect to the display");
  }

  display.setText("INIT");
  display.update();

  for (;;) {
    char displayMsg[Config::DISPLAY_MSG_SIZE];
    if (xQueueReceive(displayQueue, &displayMsg, pdMS_TO_TICKS(50))) {
      display.setText(displayMsg);
      display.update();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }

}

int main() {
  stdio_init_all();

  // Create queues
  txCanQueue = xQueueCreate(Config::CAN_TX_QUEUE_LENGTH, sizeof(CanFrame));
  rxCanQueue = xQueueCreate(Config::CAN_RX_QUEUE_LENGTH, sizeof(CanFrame));
  mpuDataQueue = xQueueCreate(Config::MPU_DATA_QUEUE_LENGTH, sizeof(Mpu6050Data));
  displayQueue = xQueueCreate(Config::DISPLAY_QUEUE_LENGTH, Config::DISPLAY_MSG_SIZE);

  if (!txCanQueue || !rxCanQueue || !mpuDataQueue || !displayQueue) {
    Logger::instance().log(LogLevel::ERROR, "Failed to create queues");
    for (;;) {
      __wfi();
    }
  }

  // Create tasks
  xTaskCreate(canTask, "CAN", Config::CAN_TASK_STACK_SIZE, nullptr, Config::CAN_TASK_PRIORITY, nullptr);
  xTaskCreate(obdTask, "OBD", Config::OBD_TASK_STACK_SIZE, nullptr, Config::OBD_TASK_PRIORITY, nullptr);
  xTaskCreate(mpu6050Task, "MPU6050", Config::MPU_TASK_STACK_SIZE, nullptr, Config::MPU_TASK_PRIORITY, nullptr);
  xTaskCreate(displayTask, "DISPLAY", Config::DISPLAY_TASK_STACK_SIZE, nullptr, Config::DISPLAY_TASK_PRIORITY, nullptr);

  // Start scheduler
  vTaskStartScheduler();

  // Should never reach here
  return 0;
}