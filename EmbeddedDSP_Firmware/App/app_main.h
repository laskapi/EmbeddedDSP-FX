#ifndef EMBEDDEDDSP_FIRMWARE_APP_MAIN_H
#define EMBEDDEDDSP_FIRMWARE_APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdio.h>

#ifdef LOG_ENABLED
#define APP_LOG(fmt, ...) printf("[APP] " fmt "\n", ##__VA_ARGS__)
#else
#define APP_LOG(fmt, ...)
#endif

/// @brief C++ application entry; wires DSP, USB control, and I2S DMA.
void app_main(void);

/// @brief USB CDC RX hook (host→device bytes).
void on_cdc_bytes(const uint8_t* Buf, uint32_t Len);

/// @brief USB CDC deinit / host gone — stops device→host audio streaming.
void on_usb_host_gone(void);

/// @brief I2S DMA half-transfer (first buffer half ready).
void on_i2s_dma_half(void);

/// @brief I2S DMA transfer complete (second buffer half ready).
void on_i2s_dma_full(void);

#ifdef __cplusplus
}
#endif

#endif // EMBEDDEDDSP_FIRMWARE_APP_MAIN_H
