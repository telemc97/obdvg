#pragma once
#include "can/UartCanBus.h"
#include <cstdint>

class ObdService {
    
    public:
        explicit ObdService(UartCanBus& bus);
        void requestPid(uint8_t pid);
        bool pollResponse(uint8_t pid, float& valueOut);

    private:
        UartCanBus& bus_;
};