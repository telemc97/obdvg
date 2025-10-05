#ifndef OBDVG_OBDSERVICE_H
#define OBDVG_OBDSERVICE_H

#include "Types.h"

// External includes
#include <cstdint>

// Internal includes
#include "can/CanFrame.h"

class ObdService {
    
    public:
        static void buildCanFrameForPID(uint8 pid, CanFrame& tx);
        static bool pollResponse(const CanFrame& rx, uint8 pid, float32& valueOut);

    private:
        static bool isValidResponse(const CanFrame& frame, uint8 requestedPid);
};

#endif //OBDVG_OBDSERVICE_H