# OBD Module (OBD)

The OBD module is responsible for the high-level logic of OBD-II communication. It handles the construction of request frames, the validation of vehicle responses, and the decoding of raw CAN data into human-readable vehicle parameters.

## Components

### 1. ObdService (`ObdService.h`, `ObdService.cpp`)
A stateless service class that manages the OBD-II protocol state machine for PID polling.

#### Key Features:
- **Request Building**: `buildCanFrameForPID()` constructs a standard OBD-II Mode 01 request frame (11-bit ID 0x7DF, 8-byte DLC).
- **Strict Response Validation**: `isValidResponse()` performs a multi-stage check on incoming frames:
    - **ID Range**: Verifies the source is a valid ECU (0x7E8 - 0x7EF).
    - **PCI Length**: Uses ISO 15765-2 Single Frame validation to ensure the reported payload length matches the expected length for the specific PID.
    - **Mode/PID Match**: Verifies the response is for Mode 01 and matches the requested PID.
    - **DLC Safety**: Ensures the physical CAN frame is large enough to contain the claimed payload, preventing out-of-bounds access.
- **Polling Logic**: `pollResponse()` coordinates validation and decoding, returning the final `float32` value.

### 2. PidDecoder (`PidDecoder.h`)
A static library of decoding functions for standard OBD-II PIDs.

#### Key Features:
- **Normalized Output**: All decoding functions return `float32` to ensure consistency and prevent overflow during intermediate calculations.
- **Wide Coverage**: Supports common PIDs including RPM, Speed, Temperatures (Coolant, Oil, Intake, Ambient), Fuel Trims, Load, and Pressures.
- **ISO Standard Formulas**: Implements standard SAE J1979 formulas with proper type casting to maintain precision (e.g., `(A-125)` for torque, `(A-40)` for temperature).

---

## Architectural Integration

- **Decoupling**: The module is stateless and relies on the `ObdTask` to provide CAN frames and manage the polling schedule.
- **Type Safety**: Strictly adheres to the `Types.h` convention, using `int32` for logic and `float32` for all physical parameters.
- **Safety First**: The module is designed to be "fail-silent"—invalid or malformed responses are rejected early in the validation phase, ensuring only high-integrity data reaches the logging and display layers.

## Protocol Specifics: ISO 15765-2 (Single Frame)

The module currently focuses on **Single Frame (SF)** communication:
1. **PCI Byte**: The first byte of the payload contains the data length (`0x02` for a request, typically `0x03` to `0x07` for a response).
2. **Mode**: Responses are identified by the request mode + `0x40` (e.g., Mode `0x01` request results in a Mode `0x41` response).
3. **Data**: Decoded based on the PID-specific formula defined in `PidDecoder`.
