# Real-Time Embedded Audio Multi-FX Processor & Spectrum Visualizer

> High-performance, bare-metal C++20 audio DSP on STM32F411, paired with a Qt 6 desktop host for USB control and real-time spectral analysis.

---

## Project Overview

Hardware–software audio system for real-time guitar multi-effects and spectrum visualization. Focus: low-latency DSP on a memory-constrained MCU, static (zero-VTable) effect pipelines, and a clean binary USB protocol shared by firmware and host.

### Key Features

* **Bare-metal audio DSP (STM32F411RE):** I2S streaming with double-buffered DMA (half / complete callbacks).
* **Static audio pipeline:** C++20 `std::variant` / `std::visit` + Concepts (`AudioEffectConcept`) instead of virtual calls on the audio path.
* **SRAM-aware effects:** e.g. Delay as mono `int16_t` with linear interpolation (~500 ms in ~48 KB).
* **USB VCP protocol:** control packets (PC → MCU) and audio frames with CRC-16 (MCU → PC).
* **Qt 6 host (C++20):** serial RX/TX (`SerialManager`), host-side FFT (`FftProcessor`), optional synthetic frame source for UI work without hardware.

---

## Repository Layout

```text
EmbeddedDSP-FX/
├── EmbeddedDSP_Firmware/     # STM32 firmware (CubeMX Core + App/ DSP & Protocol)
│   ├── Core/                 # HAL entry: main.c → app_main()
│   └── App/                  # DSP pipeline, effects, USB framing
├── EmbeddedDSP_Host/         # Qt 6 desktop application
│   ├── SerialManager.*       # QSerialPort + frame parsing / CRC
│   ├── FftProcessor.*        # Hanning window + radix-2 FFT → dB
│   ├── AudioFrameSimulator.* # Synthetic AudioFramePacket stream (host-only Demo)
│   └── mainwindow.*          # UI glue: serial and/or simulator → FFT
└── tests/                    # GoogleTest (native PC) for protocol / DSP logic