# Bluetooth Module (BT) - ELM327 Emulation

The Bluetooth module provides a high-level emulation of the ELM327 OBD-II interface over Bluetooth Low Energy (BLE). It allows standard diagnostic applications (e.g., Torque, OBDFusion) to communicate with the vehicle's CAN bus using the classic "AT" command set and hex-based OBD-II requests.

## Components

### 1. ELM327 Emulator (`ELM327.h`, `ELM327.cpp`)
A protocol translation engine that converts ASCII commands from a BLE client into binary CAN frames and vice versa.

#### Key Features:
- **AT Command Support**: Implements essential configuration commands (`ATZ`, `ATE0/1`, `ATH0/1`, `ATSH`, `ATSP`, etc.).
- **OBD-II Request Parsing**: Translates 2-character (Mode only) or 4-character (Mode + PID) hex strings into standard OBD-II CAN frames.
- **ISO 15765-2 Compliance**: Specifically handles **Single Frame (SF)** PCI bytes, ensuring correct data length extraction and masking of the PCI nibble.
- **Response Filtering**: Validates that incoming vehicle responses match the pending request's Mode and PID to prevent cross-talk from other tasks (like SD logging).
- **Extended ID Support**: Detects and handles both 11-bit (standard) and 29-bit (extended) CAN identifiers based on the `ATSH` header configuration.

#### Core Logic Flow:
1. **`processInput()`**: Cleans the raw BLE string, handles echoes, and routes to either `handleAtCommand` or `handleObdRequest`.
2. **`handleObdRequest()`**: Builds a `CanFrame` and sets the `pendingRequest_` flag.
3. **`formatCanResponse()`**: 
   - Verifies the CAN ID matches the expected response ID (Request + 8).
   - Validates the ISO 15765-2 PCI byte (`0x0N` for Single Frames).
   - Verifies the Response Mode (`Request + 0x40`) and PID.
   - Formats the result as space-separated hex bytes terminated by `\r>`.

---

## Architectural Integration

### BTstack Integration
The `ELM327` class is designed to be stateful but passive. It is driven by the `BluetoothTask`:
- **Input**: BLE `GATT_CHARACTERISTIC_WRITE` events trigger `processInput()`.
- **Output**: The `BluetoothTask` polls the `canRxQueue` and passes matching frames to `formatCanResponse()` to generate BLE notifications.

### Thread Safety
As a protocol engine, the `ELM327` instance is managed entirely within the `BluetoothTask` (specifically within the BTstack run loop context). This ensures that state changes (like changing headers with `ATSH`) are synchronized with the processing of vehicle responses.

---

## Supported Commands (Partial List)

| Command | Description |
| :--- | :--- |
| `ATZ` | Reset ELM327 (echo on, headers off, ID 7DF). |
| `ATE0 / ATE1` | Echo Off / On. |
| `ATH0 / ATH1` | Headers Off / On (show CAN ID in response). |
| `ATSH <hex>` | Set CAN Header (3 digits for 11-bit, 8 digits for 29-bit). |
| `ATDP` | Describe Protocol (returns CAN 11/500). |
| `010C` | Example OBD request (Mode 01, PID 0C - RPM). |

---

## Protocol Details: ISO 15765-2
The emulator currently supports **Single Frame (SF)** communication, which covers the majority of real-time PID data.
- **PCI Byte**: `0x0N`, where `N` is the number of data bytes following the PCI.
- **Multi-Frame**: First Frame (`0x1...`), Consecutive Frame (`0x2...`), and Flow Control (`0x3...`) are currently ignored and filtered out to maintain stability.
