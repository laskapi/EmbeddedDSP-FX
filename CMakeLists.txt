cmake_minimum_required(VERSION 3.22)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(EmbeddedDSP-FX C CXX ASM)

option(BUILD_TESTS "Build unit tests for PC" OFF)

if(BUILD_TESTS)
    message(STATUS "--- CONFIG: PC NATIVE TESTS (x86_64) ---")
    add_subdirectory(tests)
else()
    message(STATUS "--- CONFIG: STM32 FIRMWARE (ARM) ---")
    add_subdirectory(EmbeddedDSP_Firmware)
endif()
