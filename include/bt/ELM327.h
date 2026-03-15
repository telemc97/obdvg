#ifndef OBDVG_ELM327_H
#define OBDVG_ELM327_H

#include "Types.h"
#include <string>
#include "can/CanFrame.h"

/**
 * @class ELM327
 * @brief Emulates an ELM327 OBD-II interface.
 * 
 * This class parses ELM327 "AT" commands and OBD-II requests,
 * translating them into CAN frames and formatting responses.
 */
class ELM327 {
public:
    ELM327();

    /**
     * @brief Processes an incoming line of text from the Bluetooth interface.
     * @param input The raw input string.
     * @return String The response to send back to the Bluetooth client.
     */
    String processInput(const String& input);

    /**
     * @brief Translates a received CAN frame into an ELM327 ASCII response.
     * @param rx The CAN frame received from the vehicle.
     * @return String The formatted ELM327 response, or empty if frame doesn't match pending request.
     */
    String formatCanResponse(const CanFrame& rx);

    /**
     * @brief Checks if there is a pending CAN request to be sent.
     * @param[out] outFrame The CAN frame to be sent.
     * @return True if a request is pending.
     */
    bool hasPendingCanRequest(CanFrame& outFrame);

private:
    String handleAtCommand(const String& cmd);
    String handleObdRequest(const String& req);

    bool echo_ = true;
    bool linefeeds_ = false;
    bool showHeaders_ = false;
    bool pendingRequest_ = false;
    
    uint32 headerId_ = 0x7DF;
    bool isExtendedId_ = false;

    uint8 lastMode_ = 0;
    uint8 lastPid_ = 0;
    bool waitingForResponse_ = false;

    CanFrame nextFrame_ = {};
};

#endif //OBDVG_ELM327_H
