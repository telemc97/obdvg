#include "Types.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "hardware/uart.h"
#include "pico/stdlib.h"

#include "can/UartCanBus.h"

UartCanBus::UartCanBus(uart_inst_t* uart, const uint32 baud)
    : uart_(uart) {
    uart_init(uart_, baud);
}

bool UartCanBus::send(const CanFrame& frame) {
    if (frame.dlc > 8) {
        return false; // DLC too large, would overflow buffer
    }

    char buffer[32];
    int32 len;
    
    if (frame.isExtended) {
        len = std::snprintf(buffer, sizeof(buffer), "T%08X%d", frame.id, (int32)frame.dlc);
    } else {
        len = std::snprintf(buffer, sizeof(buffer), "t%03X%d", frame.id, (int32)frame.dlc);
    }
    
    if (len < 0) {
        return false; // Encoding error
    }

    for (int32 i = 0; i < frame.dlc; i++) {
        int32 result = std::snprintf(buffer + len, sizeof(buffer) - len, "%02X", frame.data[i]);
        if (result < 0) {
            return false; // Encoding error
        }
        len += result;
    }
    buffer[len++] = '\r';

    uart_write_blocking(uart_, (uint8*)buffer, len);
    return true;
}

bool UartCanBus::receive(CanFrame& frame) {
    while (uart_is_readable(uart_)) {
        char c = uart_getc(uart_);
        if (c == '\r') {
            rxBuf_[idx_] = '\0';
            idx_ = 0;

            if (std::strlen(rxBuf_) < 5) return false;

            char type = rxBuf_[0];
            if (type == 't' || type == 'T') {
                frame.isExtended = (type == 'T');
                int32 idLen = frame.isExtended ? 8 : 3;
                
                // Parse ID and DLC
                char idStr[9] = {0};
                std::memcpy(idStr, rxBuf_ + 1, idLen);
                uint32 id, dlc;
                if (std::sscanf(idStr, "%X", &id) != 1) return false;
                if (std::sscanf(rxBuf_ + 1 + idLen, "%1X", &dlc) != 1) return false;
                
                frame.id = id;
                frame.dlc = (uint8)dlc;

                if (frame.dlc > 8) return false;
                if (std::strlen(rxBuf_) != (uint32)(1 + idLen + 1 + frame.dlc * 2)) return false;

                for (int32 i = 0; i < frame.dlc; i++) {
                    uint32 val;
                    if (std::sscanf(rxBuf_ + 1 + idLen + 1 + i * 2, "%2X", &val) != 1) {
                        return false; // Invalid hex data
                    }
                    frame.data[i] = (uint8)val;
                }
                return true;
            }
            return false;
        } else if (idx_ < sizeof(rxBuf_) - 1) {
            rxBuf_[idx_++] = c;
        } else {
            idx_ = 0; // Buffer overflow, reset
        }
    }
    return false;
}

bool UartCanBus::isConnected() const {
    const char* cmd = "AT\r";
    uart_write_blocking(uart_, (const uint8*)cmd, std::strlen(cmd));

    char resp[10];
    int32 i = 0;
    uint32 start = to_ms_since_boot(get_absolute_time());
    while(to_ms_since_boot(get_absolute_time()) - start < (uint32)Config::CAN_UART_TIMEOUT_MS) { 
        if (uart_is_readable(uart_)) {
            char c = uart_getc(uart_);
            if (c == '\r' || i >= (int32)sizeof(resp)-1) {
                break;
            }
            resp[i++] = c;
        }
    }
    resp[i] = '\0';

    return std::strstr(resp, "OK") != nullptr;
}