#pragma once

#include "can/CanFrame.h"
#include <cstdint>

class ObdService {
    
    public:
        static void buildCanFrameForPID(uint8 pid, CanFrame& tx);
        static bool pollResponse(const CanFrame& rx, uint8 pid, float32& valueOut);

    private:
        static bool isValidResponse(const CanFrame& frame, uint8 requestedPid);
};