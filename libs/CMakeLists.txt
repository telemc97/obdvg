# Tell CMake where the Pico SDK is
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)

# Add Pico SDK directory
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pico-sdk)

# Add FreeRTOS directory
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel)
