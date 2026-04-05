#include "hardware/uart.h"
#include <vector>
#include <string>

static std::string rx_buffer;
static std::string tx_buffer;
static uart_inst_t uart1_inst = {1};
uart_inst_t *uart1 = &uart1_inst;

namespace MockUart {
    void inject_rx(const std::string& data) {
        rx_buffer += data;
    }
    
    std::string get_tx() {
        return tx_buffer;
    }
    
    void clear() {
        rx_buffer.clear();
        tx_buffer.clear();
    }
}

bool uart_is_readable(uart_inst_t *uart) {
    (void)uart;
    return !rx_buffer.empty();
}

uint8_t uart_getc(uart_inst_t *uart) {
    (void)uart;
    if (rx_buffer.empty()) return 0;
    uint8_t c = rx_buffer[0];
    rx_buffer.erase(0, 1);
    return c;
}

void uart_write_blocking(uart_inst_t *uart, const uint8_t *src, size_t len) {
    (void)uart;
    tx_buffer.append((const char*)src, len);
}
