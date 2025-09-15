#pragma once
#include "Types.h"
#include "can/CanFrame.h"

#include "hardware/uart.h"

class UartCanBus {
    public:
        UartCanBus(uart_inst_t* uart, uint8 txPin, uint8 rxPin, uint32 baud = 115200);
        
        bool send(const CanFrame& frame) const;
        bool receive(CanFrame& frame) const;
        
    private:
        uart_inst_t* uart_;
};