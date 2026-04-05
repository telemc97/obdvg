# SD Module (SD)

The SD module manages vehicle data persistence to an SPI-connected SD card using the FatFs library. It provides a thread-safe interface for creating, writing to, and synchronizing CSV log files.

## Components

### 1. SDLogging (`SDLogging.h`, `SDLogging.cpp`)
A thread-safe class that encapsulates the FatFs file and filesystem objects.

#### Key Features:
- **Thread Safety**: Uses a FreeRTOS Mutex (`mutex_`) to protect all filesystem and file object operations, allowing safe access from both the main logging loop and periodic synchronization timers.
- **Automated Filename Management**: `openNewLogFile()` automatically scans the SD card for existing logs and increments the filename (e.g., `log_001.csv`, `log_002.csv`) to prevent overwriting data.
- **CSV Formatting**: `logData()` formats `ObdLogMessage` objects into standardized, comma-separated ASCII lines.
- **Data Integrity**: 
    - **Byte-Write Verification**: Validates that all requested bytes are written to the disk (`bw == len`), effectively detecting "Disk Full" or hardware failure conditions.
    - **Periodic Syncing**: Supports manual flushing of data via `sync()` to ensure the directory entry and data sectors are updated on the physical medium.
- **Configurable Hardware**: Uses SPI frequencies and pin mappings defined in `Config.h` for initialization (100kHz) and high-speed operation (30MHz).

## Architectural Integration

- **Task-Based Logging**: The `SdLoggingTask` is the primary consumer of this module. It receives decoded OBD messages from a FreeRTOS queue and persists them asynchronously.
- **Resource Management**: The module handles its own initialization and cleanup, ensuring the filesystem is unmounted and the mutex is deleted upon destruction.
- **Fail-Safe Design**: The module provides public getters (`isInitialized`, `isFileOpen`) to allow the calling task to handle hardware or filesystem failures gracefully (e.g., by logging warnings instead of silently dropping data).

## Usage Example

```cpp
SDLogging logger;
if (logger.init()) {
    if (logger.openNewLogFile()) {
        logger.logData(currentObdData);
        logger.sync(); // Periodic flush
    }
}
```

## Protocol Specifics: CSV Header
The log files created by this module include a comprehensive header identifying the captured parameters:
`timestamp,rpm,speed,coolant,oil_temp,throttle,load,map,intake_temp,ambient_temp,catalyst_temp,st_trim,lt_trim,fuel_rate`
