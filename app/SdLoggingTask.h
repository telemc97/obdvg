#ifndef OBDVG_SD_LOGGING_TASK_H
#define OBDVG_SD_LOGGING_TASK_H

/**
 * @brief FreeRTOS task responsible for SD card data persistence.
 * 
 * This task consumes decoded OBD data and persists it to a CSV file 
 * on an SD card via SPI.
 * 
 * @param pvParameters Pointer to task parameters (unused).
 */
void sdLoggingTask(void* pvParameters);

#endif // OBDVG_SD_LOGGING_TASK_H
