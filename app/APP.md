# Application Module (APP)

The Application module is the central orchestrator of the OBDvg system. It manages hardware initialization, creates inter-task communication channels (FreeRTOS Queues), and launches specialized tasks for OBD communication, Bluetooth emulation, and data logging.

## System Architecture

The application follows an asynchronous, task-based architecture to ensure that high-latency operations (like SD card writes) do not block time-critical operations (like CAN bus polling).

### Core Components

#### 1. Main Entry (`main.cpp`)
- **Initialization**: Sets up standard I/O and hardware abstractions.
- **Resource Allocation**: Creates global FreeRTOS queues for inter-task communication.
- **Orchestration**: Launches the `ObdTask`, `BluetoothTask`, and `SdLoggingTask` with appropriate priorities and stack sizes.
- **Safety**: Validates that all critical resources (queues and tasks) are successfully created before starting the FreeRTOS scheduler.

#### 2. OBD Task (`ObdTask.h`, `ObdTask.cpp`)
- **Role**: The "Heart" of the system. Manages the UART-to-CAN hardware interface.
- **Polling**: Implements a round-robin PID polling engine based on `Config::PID_LOG_CONFIG`.
- **Multiplexing**: Routes incoming CAN frames to the Bluetooth module (for ELM327 emulation) and the SD logging module.
- **Simulation**: Includes a built-in simulator mode (`Config::TEST_SIMULATOR_ENABLED`) for testing without a vehicle.

#### 3. Bluetooth Task (`BluetoothTask.h`, `BluetoothTask.cpp`)
- **Role**: Provides the wireless interface for mobile diagnostic apps.
- **Stack**: Manages the BTstack (BLE) run loop and GATT services.
- **Emulation**: Integrates the `ELM327` class to translate standard AT commands into CAN requests.
- **Thread-Safety**: Uses native BTstack timers and callbacks to ensure all Bluetooth notifications are sent from the correct execution context.

#### 4. SD Logging Task (`SdLoggingTask.h`, `SdLoggingTask.cpp`)
- **Role**: Persists vehicle data to an SPI-connected SD card.
- **Data Flow**: Consumes decoded `ObdLogMessage` objects from a high-priority queue.
- **Reliability**: Implements periodic syncing and error logging to prevent data loss in the event of power failure or SD card removal.

---

## Inter-Task Communication (Queues)

The system is decoupled using three primary FreeRTOS queues:

| Queue Name | Source | Destination | Data Type | Purpose |
| :--- | :--- | :--- | :--- | :--- |
| `canTxQueue` | `BluetoothTask` | `ObdTask` | `CanFrame` | Outgoing CAN requests from ELM327. |
| `canRxQueue` | `ObdTask` | `BluetoothTask` | `CanFrame` | Incoming CAN responses for ELM327. |
| `obdDataQueue` | `ObdTask` | `SdLoggingTask` | `ObdLogMessage` | Decoded PID data for CSV logging. |

---

## Best Practices & Constraints

- **Separate Compilation**: Each task is defined in its own `.cpp` file and declared in a corresponding `.h` header. Direct inclusion of `.cpp` files is strictly prohibited.
- **Task Signatures**: All task functions must follow the standard FreeRTOS signature: `void taskName(void* pvParameters)`.
- **BTstack Context**: Any function that interacts with Bluetooth (e.g., `att_server_notify`) MUST be executed within the BTstack run loop context using timers or `async_context` workers.
- **Data Integrity**: Decoded data uses `NAN` as a default value to distinguish between a reading of `0` and a missing PID response.
- **Resource Checks**: Always verify the success of `xTaskCreate` and `xQueueCreate` to prevent silent system degradation.
