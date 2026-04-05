# Utility Module (UTIL)

The Utility module provides core system services for logging, string manipulation, and timekeeping. It is designed to be lightweight and accessible across all FreeRTOS tasks.

## Components

### 1. Logger (`Logger.h`, `Logger.cpp`)
A thread-safe singleton class responsible for console output. It features automatic line breaking to prevent long OBD strings or debug messages from overflowing standard terminal widths.

#### Key Features:
- **Singleton Pattern:** Accessed via `Logger::instance()`.
- **Level-Based Filtering:** Supports `ERROR`, `WARN`, `INFO`, and `DEBUG` levels.
- **Smart Line Breaking:** Automatically wraps long strings at a configurable character limit (default 80) without breaking words, unless a single word exceeds the limit.
- **Printf-Style Formatting:** Supports variadic templates for type-safe, formatted logging.
- **Visual Aids:** Includes `print_separator()` and `print_empty_line()` for clean log formatting.

#### Usage Example:
```cpp
// Simple log
Logger::instance().log("System initialized.");

// Formatted log with level
Logger::instance().log(LogLevel::INFO, "RPM: %d, Speed: %d", rpm, speed);

// Formatting the console
Logger::instance().print_separator();
```

---

### 2. Utils (`Utils.h`, `Utils.cpp`)
A static utility class providing system-wide helper functions.

#### Key Features:
- **Timekeeping:** Provides `getUnixTimestamp()` which returns seconds since boot (or synced RTC time) as a `uint64`.

#### Usage Example:
```cpp
uint64 now = Utils::getUnixTimestamp();
```

## Architectural Integration

- **Task Safety:** The `Logger` uses standard `printf`, which is typically thread-safe on the Pico SDK/Newlib implementation, making it suitable for use across `ObdTask`, `BluetoothTask`, and `SdLoggingTask`.
- **Type Compliance:** All implementations strictly use the project-defined types from `Types.h` (e.g., `int32`, `uint64`, `String`).
- **Dependency Minimization:** The module avoids unnecessary Pico SDK includes in headers to speed up compilation and reduce coupling.

## Configuration

The `Logger` behavior can be adjusted at runtime:
- `setCharLimit(uint32)`: Adjusts the wrap-around point for console output.
- `setLevel(LogLevel)`: Filters output to reduce console noise in production.
