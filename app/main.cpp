#include "Types.h"

#include <string>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "task.h"

#include "Config.h"
#include "can/CanFrame.h"
#include "can/UartCanBus.h"
#include "display/Display.h"
#include "mpu/Mpu6050.h"
#include "obd/ObdService.h"
#include "util/Logger.h"
#include "util/Utils.h"

#include "ObdLogTask.cpp"
#include "ObdDisplayTask.cpp"
#include "Mpu6050Task.cpp"

int main() {
    boolean stdioInititialized =  stdio_init_all();
    if (!stdioInititialized) {
        Logger::instance().log(LogLevel::ERROR, "Failed to initialize pico stdio");
        return 0;
    }

    Utils::scanI2cBus(i2c1, Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);

    Utils::initI2C(i2c1, Config::I2C_FREQUENCY, Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);

    static UartCanBus can_bus = UartCanBus(uart0, Config::CAN_BAUD);

    xTaskCreate(obdLogTask, "OBD_LOG", Config::OBD_LOG_TASK_STACK_SIZE, &can_bus, Config::OBD_LOG_TASK_PRIORITY, nullptr);
    xTaskCreate(obdDisplayTask, "OBD_DISPLAY", Config::OBD_DISPLAY_TASK_STACK_SIZE, &can_bus, Config::OBD_DISPLAY_TASK_PRIORITY, nullptr);
    xTaskCreate(mpu6050Task, "MPU6050", Config::MPU_TASK_STACK_SIZE, nullptr, Config::MPU_TASK_PRIORITY, nullptr);

    vTaskStartScheduler();

    // Should never reach here
    return 0;
}