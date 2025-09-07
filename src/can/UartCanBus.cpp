#include "can/UartCanBus.h"
#include <cstdio>

UartCanBus::UartCanBus(uart_inst_t* uart, uint8_t txPin, uint8_t rxPin, uint32_t baud)
    : uart_(uart) {
    uart_init(uart_, baud);
    gpio_set_function(txPin, GPIO_FUNC_UART);
    gpio_set_function(rxPin, GPIO_FUNC_UART);
}

bool UartCanBus::send(const CanFrame& frame) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "t%03X%d", frame.id, frame.dlc);
    for (int i = 0; i < frame.dlc; i++) {
        len += snprintf(buffer + len, sizeof(buffer) - len, "%02X", frame.data[i]);
    }
    buffer[len++] = '\r';
    uart_write_blocking(uart_, (uint8_t*)buffer, len);
    return true;
}

bool UartCanBus::receive(CanFrame& frame) {
    static char rxBuf[32];
    static int idx = 0;

    if (!uart_is_readable(uart_)) return false;

    char c = uart_getc(uart_);
    if (c == '\r') {
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