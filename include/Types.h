#ifndef OBDVG_TYPES_H
#define OBDVG_TYPES_H

#include <array>
#include <string>
#include <vector>
#include <cstdint>

// Integer types
using int8  = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

// Unsigned integer types
using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

// Floating point
using float32 = float;
using float64 = double;

// Macros for std::array, std::vector and std::string
// Note: These are kept as macros to support the project's specialized syntax
#define Array(type, size)  std::array<type, size>
#define Vector(type)       std::vector<type>
#define String             std::string

#endif //OBDVG_TYPES_H
