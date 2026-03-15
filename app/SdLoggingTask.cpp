#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "Config.h"
#include "obd/ObdLogMessage.h"
#include "sd/SDLogging.h"
#include "util/Logger.h"

extern QueueHandle_t obdDataQueue;

void sdLoggingTask(void* pvParameters) {
    (void)pvParameters;

    // 1. Initialize SPI hardware for SD Card
    spi_init(SD_SPI_INST, 1000 * 1000); // 1 MHz for initialization
    gpio_set_function(Config::SD_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(Config::SD_SPI_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(Config::SD_SPI_MISO_PIN, GPIO_FUNC_SPI);
    
    // CS Pin management
    gpio_init(Config::SD_SPI_CS_PIN);
    gpio_set_dir(Config::SD_SPI_CS_PIN, GPIO_OUT);
    gpio_put(Config::SD_SPI_CS_PIN, 1); // Deselect

    Logger::instance().log("SdLogging Task: SPI Hardware Initialized");

    // 2. Initialize SD Logging Module (Filesystem)
    static SDLogging sdLogger;
    if (sdLogger.init()) {
        if (sdLogger.openNewLogFile()) {
            Logger::instance().log("SdLogging Task: Ready to log");
        } else {
            Logger::instance().log("SdLogging Task: Failed to open log file");
        }
    } else {
        Logger::instance().log("SdLogging Task: SD init failed");
    }

    ObdLogMessage data;

    while (true) {
        // Wait for decoded data from OBD Task
        if (xQueueReceive(obdDataQueue, &data, portMAX_DELAY) == pdTRUE) {
            // Persist to SD Card
            sdLogger.logData(data);
            
            // Periodically sync/flush to avoid data loss
            static uint32 lastSync = 0;
            uint32 now = to_ms_since_boot(get_absolute_time());
            if (now - lastSync > 5000) { // Every 5 seconds
                sdLogger.sync();
                lastSync = now;
            }
        }
    }
}
