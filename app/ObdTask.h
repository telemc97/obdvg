#ifndef OBDVG_OBD_TASK_H
#define OBDVG_OBD_TASK_H

/**
 * @brief FreeRTOS task responsible for OBD-II CAN bus communication.
 * 
 * This task manages the UART-to-CAN hardware, handles periodic polling 
 * of configured PIDs, and routes incoming CAN frames to other tasks.
 * 
 * @param pvParameters Pointer to task parameters (unused).
 */
void obdTask(void* pvParameters);

#endif // OBDVG_OBD_TASK_H
