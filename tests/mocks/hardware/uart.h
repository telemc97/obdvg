#ifndef MOCK_UART_H
#define MOCK_UART_H

#include <cstdint>
#include <vector>
#include <string>

typedef struct {
    int id;
} uart_inst_t;

extern uart_inst_t *uart1;

inline void uart_init(uart_inst_t *uart, uint32_t baud) { (void)uart; (void)baud; }

// Mock functionality for test injection
namespace MockUart {
    void inject_rx(const std::string& data);
    std::string get_tx();
    void clear();
}

// We need a way to implement these in a .cpp to avoid multiple definitions
bool uart_is_readable(uart_inst_t *uart);
uint8_t uart_getc(uart_inst_t *uart);
void uart_write_blocking(uart_inst_t *uart, const uint8_t *src, size_t len);

#endif
