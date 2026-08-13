cmake_minimum_required(VERSION 3.22)

project(EmbeddedDSP C CXX ASM)

# Ustawienie standardu C/C++ dla całego projektu
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dodanie podkatalogu z firmwarem STM32
add_subdirectory(EmbeddedDSP_Firmware)