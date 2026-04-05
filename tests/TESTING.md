# Testing Strategy (OBDVG)

The OBDvg project uses a **Host-Side Unit Testing** approach. Tests are executed on the development machine (e.g., macOS/Linux) rather than the target microcontroller hardware. This ensures rapid iteration, easy debugging, and integration with standard CI/CD pipelines.

## Framework: GoogleTest (GTest)
We use the **GoogleTest** framework for all unit tests and mocking. GTest is integrated via CMake's `FetchContent` module, which automatically downloads and builds the framework during the first test run.

## Test Infrastructure

### 1. Host Build Environment
The tests are built using your system's native C++ compiler (Clang/GCC) instead of the ARM cross-compiler. This allows the code to run directly on your workstation.

### 2. Mocking Hardware (Pico SDK / FreeRTOS)
Since the host machine does not have RP2350 registers or FreeRTOS scheduling, we provide "mocks" in the `tests/mocks/` directory.
- **`hardware/uart.h`**: Simulates the UART peripheral with injectable RX/TX buffers.
- **`pico/stdlib.h`**: Stubs for Pico SDK time and system functions.

## Running Tests

To build and execute the unit tests manually on your host machine:

```bash
# Configure and build
cmake -S tests -B build_tests
cmake --build build_tests

# Run the test suite
./build_tests/obdvg_unit_tests
```

The CI/CD pipeline (`Jenkinsfile`) automatically executes these steps before every production build.

## Test Coverage Levels

### Level 1: Pure Logic (`obd/PidDecoder`)
Verifies that hex values from the vehicle are correctly converted into physical parameters (RPM, Speed, Temperature).

### Level 2: Protocol Logic (`bt/ELM327`, `obd/ObdService`)
Verifies that the ELM327 translation engine and OBD-II state machine correctly build request frames and parse vehicle responses.

### Level 3: Utility Classes (`util/Logger`)
Ensures that system-wide helpers like the thread-safe logger function correctly, specifically testing line-breaking and log-level filtering.

### Level 4: Drivers (`can/UartCanBus`)
Tests the low-level serial communication driver using hardware mocks to verify ASCII encoding/decoding and buffer safety.

## Best Practices
- **Isolation**: Each test file should ideally focus on one class or module.
- **Mocks**: When testing hardware-dependent code, use the mocks in `tests/mocks/` to simulate hardware behavior without physical devices.
- **Asserts**: Use `EXPECT_` for non-fatal failures and `ASSERT_` for fatal ones.
- **Clean State**: Ensure each test starts with a clean environment (e.g., clearing mock buffers in `SetUp()`).
