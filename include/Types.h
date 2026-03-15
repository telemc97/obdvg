#ifndef OBDVG_TYPES_H
#define OBDVG_TYPES_H

#include <array>
#include <string>
#include <vector>
#include <cstdint>

#define int8   signed char
#define int16  short
#define int32  int
#define int64  long long

// Unsigned integer types
#define uint8   unsigned char
#define uint16  unsigned short
#define uint32  unsigned int
#define uint64  unsigned long long

// Floating point
#define float32  float
#define float64  double

// Macros for std::array, std::vector and std::string
#define Array(type, size)  std::array<type, size>
#define Vector(type)       std::vector<type>
#define String             std::string

#endif //OBDVG_TYPES_H
