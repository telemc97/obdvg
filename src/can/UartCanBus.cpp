#include "Types.h"

#include "can/UartCanBus.h"
#include "can/CanFrame.h"

#include "hardware/uart.h"
#include "pico/stdlib.h"

#include <cstdio>
#include <cstdint>
       
UartCanBus::UartCanBus(uart_inst_t* uart, const uint8 txPin, const uint8 rxPin, const uint32 baud)
    : uart_(uart) {
    uart_init(uart_, baud);
    gpio_set_function(txPin, GPIO_FUNC_UART);
    gpio_set_function(rxPin, GPIO_FUNC_UART);
}

bool UartCanBus::send(const CanFrame& frame) const {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "t%03X%d", frame.id, frame.dlc);
    for (int i = 0; i < frame.dlc; i++) {
        len += snprintf(buffer + len, sizeof(buffer) - len, "%02X", frame.data[i]);
    }
    buffer[len++] = '\r';
    uart_write_blocking(uart_, (uint8*)buffer, len);
    return true;
}

bool UartCanBus::receive(CanFrame& frame) const {
    static char rxBuf[32];
    static int idx = 0;

    if (!uart_is_readable(uart_)) return false;

    if (const char c = uart_getc(uart_); c == '\r') {
        rxBuf[idx] = '\0';
        idx = 0;

        if (rxBuf[0] == 't') {
            sscanf(rxBuf + 1, "%3X%1hhu", &frame.id, &frame.dlc);
            for (int i = 0; i < frame.dlc; i++) {
                unsigned val;
                sscanf(rxBuf + 5 + i*2, "%2X", &val);
                frame.data[i] = static_cast<uint8_t>(val);
            }
            return true;
        }
        return false;
    } else {
        if (idx < static_cast<int>(sizeof(rxBuf)) - 1) rxBuf[idx++] = c;
    }
    return false;
}
