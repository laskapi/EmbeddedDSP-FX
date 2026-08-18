# ==============================================================================
# 1. BARE-METAL SYSTEM SETUP (Must be defined before project())
# ==============================================================================
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

cmake_minimum_required(VERSION 3.22)

# Standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME EmbeddedDSP_Firmware)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME} C CXX ASM)

# ==============================================================================
# 2. HARDWARE & ARCHITECTURE FLAGS
# ==============================================================================
add_compile_options(
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
)

add_link_options(
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        --specs=nano.specs
        --specs=nosys.specs
        -Wl,--no-warn-rwx-segments
)

# ==============================================================================
# 3. EXECUTABLE TARGET & SOURCES
# ==============================================================================
add_executable(${CMAKE_PROJECT_NAME}
        # C++ Application Logic
        App/app_main.cpp
        App/app_main.h
        App/DelayEffect.h
        App/DynamicAudioPipeline.h
        App/AudioEffectConcept.h
        App/OverdriveEffect.h
        App/Protocol.h
        App/LockFreeQueue.h
        App/ProtocolParser.h

        # USB Device Stack
        USB_DEVICE/Target/usbd_conf.c
        USB_DEVICE/App/usb_device.c
        USB_DEVICE/App/usbd_desc.c
        USB_DEVICE/App/usbd_cdc_if.c

        # Core / Hardware Init
        Core/Src/main.c
        Core/Src/gpio.c
        Core/Src/dma.c
        Core/Src/i2c.c
        Core/Src/i2s.c
        Core/Src/stm32f4xx_it.c
        Core/Src/stm32f4xx_hal_msp.c
        Core/Src/sysmem.c
        Core/Src/syscalls.c

        # Startup Assembly File
        startup_stm32f411xe.s
)

# Prevent CMake from appending .exe extension on Windows host
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES SUFFIX ".elf")

# ==============================================================================
# 4. LINKER SCRIPT BINDING
# ==============================================================================
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
        "-T${CMAKE_CURRENT_SOURCE_DIR}/STM32F411XX_FLASH.ld"
)

# ==============================================================================
# 5. INCLUDES & LIBRARIES
# ==============================================================================
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        App
        Core/Inc
        USB_DEVICE/App
        USB_DEVICE/Target
)

# Include CubeMX auto-generated target
add_subdirectory(cmake/stm32cubemx)

# Plain signature link to match CubeMX CMake configuration
target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx
)

# Workaround for unwanted libob.a dependency in C++ builds
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)