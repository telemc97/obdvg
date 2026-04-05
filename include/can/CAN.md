# CAN Module (CAN)

The CAN module provides a low-level driver for communicating with the vehicle via a Waveshare TTL UART-to-CAN bridge. It abstracts the ASCII-based UART protocol into a structured `CanFrame` interface.

## Components

### 1. UartCanBus (`UartCanBus.h`, `UartCanBus.cpp`)
The primary driver for the UART-to-CAN adapter. It handles the low-level serial communication, ASCII encoding/decoding, and connection verification.

#### Key Features:
- **ASCII Protocol Encoding**: Converts `CanFrame` objects into ASCII strings for the adapter:
    - Standard IDs (11-bit): `t<3-digit hex ID><1-digit DLC><DATA>\r`
    - Extended IDs (29-bit): `T<8-digit hex ID><1-digit DLC><DATA>\r`
- **Non-Blocking Reception**: Uses an internal buffer (`rxBuf_`) to process incoming UART data character-by-character, returning complete frames only when the `\r` terminator is received.
- **Buffer Safety**: Includes strict DLC validation (max 8 bytes) and buffer-limit checks to prevent overflows during encoding and decoding.
- **Connection Diagnostics**: Provides an `isConnected()` method that uses an "AT" command to verify the adapter is responsive and configured correctly.

#### Key Driver Parameters:
- **Default Baud Rate**: 115,200 bps.
- **Timeout**: Managed by `Config::CAN_UART_TIMEOUT_MS`.

### 2. CanFrame (`CanFrame.h`)
A lightweight data structure representing a single CAN message.

#### Structure:
- `uint32 id`: The CAN identifier (Standard or Extended).
- `uint8 dlc`: Data Length Code (0-8 bytes).
- `bool isExtended`: Flag indicating if the ID is a 29-bit extended identifier.
- `Array(uint8, 8) data`: The raw CAN payload.

---

## Architectural Integration

- **Task Interaction**: The `UartCanBus` is instantiated within the `ObdTask`. It is the primary data source for the system.
- **Queue Interop**: Frames received by this module are forwarded to the `canRxQueue` for consumption by the Bluetooth/ELM327 task.
- **Flow Control**: The driver is passive; it relies on the `ObdTask` to poll for data and handle the timing of outgoing requests.

## Protocol Specifics

The driver interacts with the hardware adapter using a simple ASCII command set.
- **Sending**: The `send()` method constructs the command string and uses `uart_write_blocking`.
- **Receiving**: The `receive()` method drains the UART hardware FIFO into the class's `rxBuf_` until a carriage return is found, at which point it parses the frame.
- **Validation**: All incoming frames must strictly match the expected length based on their DLC to be considered valid.
