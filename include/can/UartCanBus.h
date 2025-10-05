#ifndef OBDVG_UARTCANBUS_H
#define OBDVG_UARTCANBUS_H

#include "Types.h"

// Pico SDK includes
#include "hardware/uart.h"

// Internal includes
#include "can/CanFrame.h"

class UartCanBus {
    public:
        /**
         * @brief Construct a new Uart Can Bus object
         * 
         * The protocol for sending and receiving CAN frames is as follows:
         * Send:    t<3-digit hex ID><1-digit DLC><2-digit hex data bytes>\r
         * Receive: t<3-digit hex ID><1-digit DLC><2-digit hex data bytes>\r
         * 
         * @param uart The UART instance to use
         * @param baud The baud rate for the UART
         */
        explicit UartCanBus(uart_inst_t* uart, uint32 baud = 115200);
        
        boolean send(const CanFrame& frame);
        boolean receive(CanFrame& frame);
        boolean isConnected() const;

    private:
        uart_inst_t* uart_;
        char rxBuf_[32]{};
        int idx_ = 0;
};

#endif //OBDVG_UARTCANBUS_H
