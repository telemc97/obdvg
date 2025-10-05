#ifndef OBDVG_PIDDECODER_H
#define OBDVG_PIDDECODER_H

#include "Types.h"

class PidDecoder {
    public:
        static float decodeRpm(uint8 const A, uint8 const B) { return ((A << 8) | B) / 4.0f; }
        static uint8 decodeSpeed(uint8 const A) { return A; }
        static int8 decodeTemp(uint8 const A) { return static_cast<int8>(A) - 40; }
};

#endif //OBDVG_PIDDECODER_H