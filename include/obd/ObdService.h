#ifndef OBDVG_OBDSERVICE_H
#define OBDVG_OBDSERVICE_H

#include "Types.h"

// External includes
#include <cstdint>

// Internal includes
#include "can/CanFrame.h"

#include "obd/ObdPids.h"

/**
 * @class ObdService
 * @brief Provides static methods to handle OBD-II communications.
 *
 * This class includes functions to build CAN frames for OBD-II PID requests
 * and to process the responses from the vehicle.
 */
class ObdService {
public:
    /**
     * @brief Constructs a CAN frame for a specific OBD-II PID request.
     * @param pid The Parameter ID (PID) to request.
     * @param[out] tx The CAN frame to be populated with the request data.
     */
    static void buildCanFrameForPID(ObdPid pid, CanFrame &tx);

    /**
     * @brief Polls and decodes a response from a received CAN frame.
     * @param rx The received CAN frame.
     * @param pid The PID that was requested.
     * @param[out] valueOut The decoded value from the response.
     * @return True if the response is valid and successfully decoded, false otherwise.
     */
    static bool pollResponse(const CanFrame &rx, ObdPid pid, float32 &valueOut);

private:
    /**
     * @brief Validates if a received CAN frame is a valid response for the requested PID.
     * @param frame The received CAN frame.
     * @param requestedPid The PID that was requested.
     * @return True if the frame is a valid response, false otherwise.
     */
    static bool isValidResponse(const CanFrame &frame, ObdPid requestedPid);
};

#endif //OBDVG_OBDSERVICE_H