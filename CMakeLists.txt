cmake_minimum_required(VERSION 3.22)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(EmbeddedDSP-FX C CXX ASM)

# Domyślnie budujemy firmware dla STM32.
# Jeśli włączymy flagę BUILD_TESTS=ON, budujemy testy na PC.
option(BUILD_TESTS "Buduj unit testy na PC" OFF)

if(BUILD_TESTS)
    message(STATUS "--- KONFIGURACJA: TESTY NATYWNE PC (x86_64) ---")
    add_subdirectory(tests)
else()
    message(STATUS "--- KONFIGURACJA: FIRMWARE STM32 (ARM) ---")
    add_subdirectory(EmbeddedDSP_Firmware)
endif()