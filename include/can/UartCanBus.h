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
         * @brief Construct a new UartCanBus object.
         * This class handles communication with a UART-to-CAN bus adapter.
         * The protocol for sending and receiving CAN frames is an ASCII format:
         * - Send/Receive: t<3-digit hex ID><1-digit DLC><DATA>\r
         *
         * @param uart The UART instance to use (e.g., uart0).
         * @param baud The baud rate for the UART communication.
         */
        explicit UartCanBus(uart_inst_t* uart, uint32 baud = 115200);

        /**
         * @brief Sends a CAN frame over the UART bus.
         * The frame is encoded into the required ASCII format.
         * @param frame The CanFrame object to send.
         * @return True if the frame was encoded and sent successfully, false on encoding error.
         */
        boolean send(const CanFrame& frame);

        /**
         * @brief Receives and decodes a CAN frame from the UART bus.
         * This is a non-blocking function that must be called repeatedly.
         * It processes incoming UART data and only returns true when a complete,
         * valid CAN frame has been successfully parsed.
         * @param[out] frame Reference to a CanFrame object to store the received frame.
         * @return True if a complete frame was received, false otherwise.
         */
        boolean receive(CanFrame& frame);

        /**
         * @brief Checks if the UART-to-CAN adapter is connected and responsive.
         * Sends an "AT" command and waits for an "OK" response.
         * @return True if the adapter responds correctly, false otherwise.
         */
        boolean isConnected() const;

    private:
        uart_inst_t* uart_;
        char rxBuf_[32]{};
        int idx_ = 0;
};

#endif //OBDVG_UARTCANBUS_H
