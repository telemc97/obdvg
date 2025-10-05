#ifndef OBDVG_CANFRAME_H
#define OBDVG_CANFRAME_H

#include "Types.h"

#include <array>

struct CanFrame {
    uint32 id{0};
    uint8 dlc{0};
    Array(uint8, 8) data{};
};

#endif //OBDVG_CANFRAME_H