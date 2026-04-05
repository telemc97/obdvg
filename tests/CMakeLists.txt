cmake_minimum_required(VERSION 3.14)
project(obdvg_tests)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB_RECURSE UNIT_TEST_FILES CONFIGURE_DEPENDS "*.test.cpp")

add_executable(obdvg_unit_tests
  ${UNIT_TEST_FILES}
  mocks/MockUart.cpp
  ../src/bt/ELM327.cpp
  ../src/obd/ObdService.cpp
  ../src/util/Logger.cpp
  ../src/can/UartCanBus.cpp
)

target_include_directories(obdvg_unit_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

target_link_libraries(obdvg_unit_tests
  GTest::gtest_main
)

target_compile_options(obdvg_unit_tests PRIVATE --coverage -O0 -g)
target_link_options(obdvg_unit_tests PRIVATE --coverage)

include(GoogleTest)
gtest_discover_tests(obdvg_unit_tests)
