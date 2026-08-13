# Real-Time Embedded Audio Multi-FX Processor & Spectrum Visualizer

> High-performance, bare-metal C++20 audio signal processing engine running on STM32F411, integrated with a Qt 6 desktop application for real-time control and spectral analysis.

---

## 📌 Project Overview

This project is a hardware-software hybrid audio system designed for real-time guitar multi-effects processing and audio signal visualization. It demonstrates high-performance, low-latency DSP engineering without virtual function call overhead, combined with optimized memory architecture for microcontrollers.

### Key Features
* **Bare-Metal Audio DSP (STM32F411RE):** I2S audio stream processing via double-buffered DMA (Ping-Pong scheme).
* **Zero-Overhead Static Pipeline:** Uses C++20 `std::variant`, `std::visit`, and Concepts for compile-time polymorphism instead of dynamic VTable calls.
* **SRAM-Optimized Effects:** Delay buffer engineered to fit tight MCU memory constraints (using `int16_t` mono conversion and fractional linear interpolation).
* **Dual-Bus Communication Architecture:**
   * **USB VCP (Native):** Fast binary link for real-time parameter control and streaming raw audio frames to PC.
   * **UART / ST-LINK:** Dedicated system logging channel for diagnostic output.
* **Desktop Control & Visualizer (Qt 6 / C++20):** Graphical user interface for effect management and real-time Fast Fourier Transform (FFT) spectrum plot.

---

## 🛠 Hardware & System Architecture

| Component | Specification |
| :--- | :--- |
| **Microcontroller** | STM32F411RE (ARM Cortex-M4 @ 96 MHz, Hardware FPU) |
| **Audio Interface** | External Codec / ADC & DAC via I2S3 |
| **Memory Constraint** | 128 KB SRAM, 512 KB Flash |
| **DMA Strategy** | Circular Buffer (Half-Transfer & Transfer-Complete interrupts) |
| **Desktop Stack** | Qt 6, C++20/C++23, Async/Coroutines, QCustomPlot |

### Signal Flow Architecture

    +---------------------------------------------------------------------------------+
    |                         Guitar / Analog Audio Input                             |
    +---------------------------------------------------------------------------------+
                                             |
                                             v
    +---------------------------------------------------------------------------------+
    | STM32F411 Discovery (Embedded Firmware)                                         |
    |                                                                                 |
    |  +--------------------+      +-----------------------------------------------+  |
    |  | I2S / DMA Hardware | ---> | DynamicAudioPipeline<2>                       |  |
    |  | Ring Buffers       |      | - Slot 0: DelayEffect (int16_t mono, 500ms)   |  |
    |  +--------------------+      | - Slot 1: OverdriveEffect (Soft-Clipping, EQ) |  |
    |                              +-----------------------------------------------+  |
    +----------------------------------------+----------------------------------------+
                                             |
                                             | USB VCP (Virtual COM Port)
                                             v
    +---------------------------------------------------------------------------------+
    | Qt 6 Desktop Application (PC Controller)                                        |
    |                                                                                 |
    |  +----------------------------------+     +----------------------------------+  |
    |  | Effect Controls & Preset Manager |     | Real-Time FFT Spectrum Visualizer|  |
    |  +----------------------------------+     +----------------------------------+  |
    +---------------------------------------------------------------------------------+

---

## ⚡ DSP & Embedded Innovations

### 1. Zero-VTable Static Polymorphism
To avoid pipeline flushes and memory-jump penalties (`BLX` instructions) on ARM Cortex-M4 during high-frequency DMA callbacks, the pipeline replaces virtual methods with C++20 static interfaces (`AudioEffectConcept`).

### 2. SRAM Optimization Strategy
Storing 1 second of 32-bit float stereo audio at 48 kHz requires **384 KB of RAM** (exceeding the entire 128 KB SRAM of the STM32F411).
* **Optimization:** Input signals are downmixed to 16-bit mono (`int16_t`).
* **Result:** A 24,000-sample circular buffer provides **500 ms of clean delay** using only **48 KB RAM** with ~96 dB dynamic range.

---

## 🚀 Getting Started & Build Instructions

### Prerequisites
* **ARM Embedded Toolchain:** `arm-none-eabi-gcc` (supporting C++20)
* **Build System:** CMake 3.22+ & Ninja / Make
* **Desktop:** Qt 6.x SDK with C++20 compiler (GCC/Clang/MSVC)

### Building Embedded Firmware
1. `mkdir build && cd build`
2. `cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake ..`
3. `make -j4`

---

## 📄 License & Contact
Developed as a showcase project for high-performance Embedded Audio Engineering and C++20 System Architecture.