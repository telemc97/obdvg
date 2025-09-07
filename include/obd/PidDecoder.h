#pragma once
#include <cstdint>

class PidDecoder {
    public:
        static float decodeRpm(uint8_t A, uint8_t B) { return ((A << 8) | B) / 4.0f; }
        static uint8_t decodeSpeed(uint8_t A) { return A; }
        static int8_t decodeTemp(uint8_t A) { return static_cast<int8_t>(A) - 40; }
};