#pragma once

#include <array>
#include <string>

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

// Boolean (C++ already has bool, but sometimes for C interop)
#define boolean  unsigned char

// Macros for std::array and std::string
#define Array(type, size)  std::array<type, size>
#define String             std::string