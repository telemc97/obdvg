#pragma once
#include "hardware/uart.h"

class UartCanBus {
    public:
        UartCanBus(uart_inst_t* uart, uint8_t txPin, uint8_t rxPin, uint32_t baud = 115200);
        bool send(const CanFrame& frame);
        bool receive(CanFrame& frame);
        
    private:
        uart_inst_t* uart_;
};