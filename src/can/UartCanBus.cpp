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

boolean UartCanBus::send(const CanFrame& frame) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "t%03X%d", frame.id, frame.dlc);
    if (len < 0) {
        return false; // Encoding error
    }

    for (int i = 0; i < frame.dlc; i++) {
        int result = snprintf(buffer + len, sizeof(buffer) - len, "%02X", frame.data[i]);
        if (result < 0) {
            return false; // Encoding error
        }
        len += result;
    }
    buffer[len++] = '\r';

    uart_write_blocking(uart_, (uint8*)buffer, len);
    return true;
}

boolean UartCanBus::receive(CanFrame& frame) {
    while (uart_is_readable(uart_)) {
        char c = uart_getc(uart_);
        if (c == '\r') {
            rxBuf_[idx_] = '\0';
            idx_ = 0;

            if (rxBuf_[0] == 't' && strlen(rxBuf_) >= 5) {
                if (sscanf(rxBuf_ + 1, "%3X%1hhu", &frame.id, &frame.dlc) != 2 || frame.dlc > 8) {
                    return false; // Invalid format or DLC
                }

                if (strlen(rxBuf_) != 5 + frame.dlc * 2) {
                    return false; // Incorrect length
                }

                for (int i = 0; i < frame.dlc; i++) {
                    unsigned val;
                    if (sscanf(rxBuf_ + 5 + i * 2, "%2X", &val) != 1) {
                        return false; // Invalid hex data
                    }
                    frame.data[i] = static_cast<uint8>(val);
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

boolean UartCanBus::isConnected() const {
    const char* cmd = "AT\r";
    uart_write_blocking(uart_, (const uint8*)cmd, strlen(cmd));

    char resp[10];
    int i = 0;
    uint32 start = to_ms_since_boot(get_absolute_time());
    while(to_ms_since_boot(get_absolute_time()) - start < 100) { // 100ms timeout
        if (uart_is_readable(uart_)) {
            char c = uart_getc(uart_);
            if (c == '\r' || i >= sizeof(resp)-1) {
                break;
            }
            resp[i++] = c;
        }
    }
    resp[i] = '\0';

    return strstr(resp, "OK") != nullptr;
}