cmake_minimum_required(VERSION 3.13)

project(obdvg C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# FreeRTOSConfig.h Configuration
add_library(freertos_config INTERFACE)

target_include_directories(freertos_config SYSTEM
INTERFACE
    include
)

target_compile_definitions(freertos_config
  INTERFACE
    projCOVERAGE_TEST=0
)

# Tell CMake where the Pico SDK is
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/libs/pico-sdk)

# Add Pico SDK directory
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/pico-sdk)

# Add FreeRTOS directory
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs//FreeRTOS-Kernel)

# Define executable
add_executable(obdvg
    app/main.cpp
)

# Pico SDK Configuration
set(PICO_BOARD pico2_w)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

pico_sdk_init()

# Include headers
target_include_directories(obdvg PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/libs/FreeRTOS-Kernel)

# Include FreeRTOS
include(FreeRTOS_Kernel_import.cmake)

# Link with Pico SDK + FreeRTOS
target_link_libraries(obdvg
    pico_stdlib
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)

# Enable USB/serial stdio
pico_enable_stdio_usb(obdvg 1)
pico_enable_stdio_uart(obdvg 0)

# Generate UF2, bin, etc.
pico_add_extra_outputs(obdvg)
