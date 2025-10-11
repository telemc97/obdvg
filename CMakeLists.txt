cmake_minimum_required(VERSION 3.13)

if (DEBUG)
  set(PICO_DEOPTIMIZED_DEBUG 1)
  set(DPICO_BOARD pico2_w)
  set(DCMAKE_BUILD_TYPE Debug)
endif ()

  # pico-sdk before project
# Setting variables
set(PICO_BOARD pico2_w)
set(PICO_PLATFORM rp2350-arm-s)
set(PICO_FLASH_SIZE 2048)
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/libs/pico-sdk)
include(pico_sdk_import.cmake)

project(obdvg C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# freeRTOS
# Setting variables and FreeRTOSConfig.h
set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)

set(FREERTOS_PORT "GCC_ARM_CM33_NONSECURE" CACHE STRING "" FORCE)

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE include)
target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)

# Pico SDK Configuration
pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/FreeRTOS-Kernel)
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/libs/FreeRTOS-Kernel)
include(FreeRTOS_Kernel_import.cmake)

file(GLOB obdvg_MAIN CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/app/main.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/**/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/FreeRTOSHooks.c"
)

# Define executable
add_executable(obdvg
    ${obdvg_MAIN}
)

set_target_properties(obdvg PROPERTIES
        OUTPUT_NAME "obdvg"
        SUFFIX ".elf"
)

# Include headers
target_include_directories(obdvg PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_include_directories(obdvg SYSTEM PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/libs/FreeRTOS-Kernel
    ${CMAKE_CURRENT_LIST_DIR}/libs/pico-sdk
)

target_link_libraries(obdvg
    pico_stdlib
    hardware_i2c
    hardware_gpio
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)

# Enable USB/serial stdio
pico_enable_stdio_usb(obdvg 1)
pico_enable_stdio_uart(obdvg 1)

pico_add_extra_outputs(obdvg)