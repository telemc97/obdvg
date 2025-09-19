#pragma once
#include "Types.h"
#include "can/CanFrame.h"

#include "hardware/uart.h"

class UartCanBus {
    public:
        UartCanBus(uart_inst_t* uart, uint8 txPin, uint8 rxPin, uint32 baud = 115200);
        
        boolean send(const CanFrame& frame) const;
        boolean receive(CanFrame& frame) const;
        
    private:
        uart_inst_t* uart_;
};