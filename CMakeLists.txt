cmake_minimum_required(VERSION 3.22)
project(EmbeddedDSP_Workspace LANGUAGES C CXX ASM)

# Zabezpieczenie: Jeśli CMAKE_SYSTEM_NAME to Generic, oznacza to, że budujemy na STM32
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    message(STATUS "--- CONFIG: STM32 FIRMWARE (ARM) ---")
    add_subdirectory(EmbeddedDSP_Firmware)

else()
    message(STATUS "--- CONFIG: PC DESKTOP (Host & Tests) ---")
    # Te dwa podfoldery potrzebują normalnego kompilatora PC
    add_subdirectory(EmbeddedDSP_Host)
    add_subdirectory(tests)
endif()
