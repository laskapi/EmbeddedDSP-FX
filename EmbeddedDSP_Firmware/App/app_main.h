#ifndef EMBEDDEDDSP_FIRMWARE_APP_MAIN_H
#define EMBEDDEDDSP_FIRMWARE_APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdio.h>

#ifdef APP_LOG_ENABLED
#define APP_LOG(fmt, ...) printf("[APP] " fmt "\n", ##__VA_ARGS__)
#else
#define APP_LOG(fmt, ...)
#endif

/// @brief C++ application entry; wires DSP, USB control, and I2S DMA.
void app_main(I2S_HandleTypeDef* audio_i2s);

/// @brief I2S DMA half-transfer callback (first buffer half ready).
void app_audio_half_transfer_cb(void);

/// @brief I2S DMA complete callback (second buffer half ready).
void app_audio_transfer_complete_cb(void);

#ifdef __cplusplus
}
#endif

#endif // EMBEDDEDDSP_FIRMWARE_APP_MAIN_H
