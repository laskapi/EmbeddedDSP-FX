#ifndef EMBEDDEDDSP_FIRMWARE_AUDIO_DAC_H
#define EMBEDDEDDSP_FIRMWARE_AUDIO_DAC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the CS43L22 Audio DAC via I2C (Internal DAC).
 */
int AudioDAC_Init(void);

/**
 * @brief Initializes the WM8960 Audio Codec via I2C (External ADC/DAC).
 */
int WM8960_Init(void);

#ifdef __cplusplus
}
#endif

#endif // EMBEDDEDDSP_FIRMWARE_AUDIO_DAC_H