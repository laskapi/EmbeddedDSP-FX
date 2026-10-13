cmake_minimum_required(VERSION 3.22)

option(BUILD_TESTS "Build unit tests for PC" OFF)

if(NOT BUILD_TESTS)
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

project(EmbeddedDSP-FX C CXX ASM)

if(BUILD_TESTS)
    message(STATUS "--- CONFIG: PC NATIVE TESTS (x86_64) ---")
    add_subdirectory(tests)
else()
    message(STATUS "--- CONFIG: STM32 FIRMWARE (ARM) ---")
    add_subdirectory(EmbeddedDSP_Firmware)
endif()
