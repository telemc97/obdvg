#pragma once
#include <cstdint>
#include <array>

struct CanFrame {
    uint32_t id{0};
    uint8_t dlc{0};
    std::array<uint8_t, 8> data{};
};