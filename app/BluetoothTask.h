#ifndef OBDVG_BLUETOOTH_TASK_H
#define OBDVG_BLUETOOTH_TASK_H

/**
 * @brief FreeRTOS task responsible for BLE ELM327 emulation.
 * 
 * This task manages the BTstack initialization, the ELM327 protocol emulator, 
 * and handles data exchange between the BLE interface and the OBD task.
 * 
 * @param pvParameters Pointer to task parameters (unused).
 */
void bluetoothTask(void* pvParameters);

#endif // OBDVG_BLUETOOTH_TASK_H
