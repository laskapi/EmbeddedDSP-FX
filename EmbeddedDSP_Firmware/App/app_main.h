#ifndef EMBEDDEDDSP_FIRMWARE_APP_MAIN_H
#define EMBEDDEDDSP_FIRMWARE_APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../Core/Inc/main.h"

/**
 * @brief Main entry point for the C++ application domain.
 * @param audio_i2s Pointer to the HAL I2S handle for DMA configuration.
 */
void app_main(I2S_HandleTypeDef* audio_i2s);

/**
 * @brief Callback triggered when the DMA receives/transmits the first half of the buffer.
 */
void app_audio_half_transfer_cb(void);

/**
 * @brief Callback triggered when the DMA receives/transmits the second half of the buffer.
 */
void app_audio_transfer_complete_cb(void);

#ifdef __cplusplus
}
#endif

#endif // EMBEDDEDDSP_FIRMWARE_APP_MAIN_H