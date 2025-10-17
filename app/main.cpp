#include "Types.h"

#include <string>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
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
void canTask(void *pvParameters) {
    UartCanBus canBus(uart0, Config::CAN_BAUD);

    CanFrame txFrame;
    CanFrame rxFrame;

    boolean isConnected = true;

    for (;;) {

        boolean currentlyConnected = canBus.isConnected();

        if (currentlyConnected != isConnected) {
            if (currentlyConnected) {
                Logger::instance().log(LogLevel::INFO, "CAN driver is connected");
            } else {
                Logger::instance().log(LogLevel::ERROR, "CAN driver is disconnected");
            }
            isConnected = currentlyConnected;
        }

        if (!isConnected) {
            vTaskDelay(pdMS_TO_TICKS(Config::CAN_V_TASK_DELAY));
            continue;
        }

        if (xQueueReceive(txCanQueue, &txFrame, pdMS_TO_TICKS(10))) {
            boolean txStatus = canBus.send(txFrame);
            if (!txStatus) {
                Logger::instance().log(LogLevel::ERROR, "Can send failed");
            }
        }

        boolean rxStatus = canBus.receive(rxFrame);

        if (rxStatus) {
            if (xQueueSend(rxCanQueue, &rxFrame, 0) != pdPASS) {
                Logger::instance().log(LogLevel::ERROR, "Failed to send received CAN frame to queue");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(Config::CAN_V_TASK_DELAY));
    }

}

// OBD TASK
void obdTask(void *pvParameters) {

    for (;;) {

        CanFrame txFrame;
        ObdService::buildCanFrameForPID(PID_ENGINE_RPM, txFrame);

        if (xQueueSend(txCanQueue, &txFrame, portMAX_DELAY) != pdPASS) {
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
            Logger::instance().log(LogLevel::ERROR, "Timeout waiting for OBD response for PID: %d", PID_ENGINE_RPM);
        }

        vTaskDelay(pdMS_TO_TICKS(Config::OBD_V_TASK_DELAY));
    }
}

// MPU6050 TASK
void mpu6050Task(void *pvParameters) {
    Mpu6050 mpu6050(i2c1);

    Mpu6050Data mpu_6050data{};

    boolean isConnected = true;

    for (;;) {

        boolean currentlyConnected = mpu6050.isConnected();

        if (currentlyConnected != isConnected) {
            if (currentlyConnected) {
                Logger::instance().log(LogLevel::INFO, "MPU is connected");
                mpu6050.init();
            } else {
                Logger::instance().log(LogLevel::ERROR, "MPU is disconnected");
            }
            isConnected = currentlyConnected;
        }

        if (!isConnected) {
            vTaskDelay(pdMS_TO_TICKS(Config::MPU_V_TASK_DELAY));
            continue;
        }

        mpu6050.readData(mpu_6050data);
        if (xQueueSend(mpuDataQueue, &mpu_6050data, 0) != pdPASS) {
            Logger::instance().log(LogLevel::ERROR, "Failed to send MPU data to queue");
        }
        vTaskDelay(pdMS_TO_TICKS(Config::MPU_V_TASK_DELAY));
    }
}

// DISPLAY TASK
void displayTask(void *pvParameters) {
    Display display(i2c1);

    boolean isConnected = true;

    for (;;) {

        boolean currentlyConnected = display.isConnected();

        if (currentlyConnected != isConnected) {
            if (currentlyConnected) {
                Logger::instance().log(LogLevel::INFO, "Display is connected");
            } else {
                Logger::instance().log(LogLevel::ERROR, "Display is disconnected");
            }
            isConnected = currentlyConnected;
        }

        if (!isConnected) {
            vTaskDelay(pdMS_TO_TICKS(Config::DISPLAY_V_TASK_DELAY));
            continue;
        }
        display.init();
        char displayMsg[Config::DISPLAY_MSG_SIZE];
        if (xQueueReceive(displayQueue, &displayMsg, pdMS_TO_TICKS(50))) {
            display.setText(displayMsg);
            display.update();
        }
        vTaskDelay(pdMS_TO_TICKS(Config::DISPLAY_V_TASK_DELAY));
    }

}

uint32 initI2C(i2c_inst_t* i2c_instance, uint32 freq, uint8 sdaPin, uint8 sclPin) {
    const uint32 ret = i2c_init(i2c_instance, freq);
    gpio_set_function(sdaPin, GPIO_FUNC_I2C);
    gpio_pull_up(sdaPin);
    gpio_set_function(sclPin, GPIO_FUNC_I2C);
    gpio_pull_up(sclPin);
    return  ret;
}

int main() {
    boolean stdioInititialized =  stdio_init_all();
    if (!stdioInititialized) {
        Logger::instance().log(LogLevel::ERROR, "Failed to initialize pico stdio");
        return 0;
    }

    Utils::scanI2cBus(i2c1, Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);

    initI2C(i2c1, Config::I2C_FREQUENCY, Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);

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

    xTaskCreate(canTask, "CAN", Config::CAN_TASK_STACK_SIZE, nullptr, Config::CAN_TASK_PRIORITY, nullptr);
    xTaskCreate(obdTask, "OBD", Config::OBD_TASK_STACK_SIZE, nullptr, Config::OBD_TASK_PRIORITY, nullptr);
    xTaskCreate(mpu6050Task, "MPU6050", Config::MPU_TASK_STACK_SIZE, nullptr, Config::MPU_TASK_PRIORITY, nullptr);
    xTaskCreate(displayTask, "DISPLAY", Config::DISPLAY_TASK_STACK_SIZE, nullptr, Config::DISPLAY_TASK_PRIORITY, nullptr);

    vTaskStartScheduler();

    // Should never reach here
    return 0;
}