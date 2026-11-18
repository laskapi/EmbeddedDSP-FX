#include "audio_dac.h"
#include "main.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_utils.h"

#define CS43L22_I2C_ADDR 0x94

/**
 * @brief Helper function for I2C register writes using LL drivers.
 */
static int I2C_Write_LL(uint8_t reg, uint8_t value) {
    uint32_t timeout = 10000;
    
    while (LL_I2C_IsActiveFlag_BUSY(I2C1) && --timeout);
    if (timeout == 0) return -1;

    LL_I2C_GenerateStartCondition(I2C1);
    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_SB(I2C1) && --timeout);
    if (timeout == 0) return -1;

    LL_I2C_TransmitData8(I2C1, CS43L22_I2C_ADDR);
    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1) && --timeout);
    if (timeout == 0) return -1;
    LL_I2C_ClearFlag_ADDR(I2C1);

    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_TXE(I2C1) && --timeout);
    LL_I2C_TransmitData8(I2C1, reg);

    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_TXE(I2C1) && --timeout);
    LL_I2C_TransmitData8(I2C1, value);

    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_BTF(I2C1) && --timeout);
    LL_I2C_GenerateStopCondition(I2C1);

    return 0;
}

int AudioDAC_Init(void) {
    // Hardware Reset
    LL_GPIO_SetOutputPin(Audio_RST_GPIO_Port, Audio_RST_Pin);
    HAL_Delay(10);

    // Required initialization sequence for CS43L22
    if (I2C_Write_LL(0x00, 0x99) != 0) return -1;
    I2C_Write_LL(0x47, 0x80);
    I2C_Write_LL(0x32, 0xBB);
    I2C_Write_LL(0x32, 0x3B);
    I2C_Write_LL(0x00, 0x00);

    // Power Down for configuration
    I2C_Write_LL(0x02, 0x01);

    // Interface Control: I2S Philips, 16-bit
    I2C_Write_LL(0x06, 0x07);

    // Set master volume to a safe level (0xD0 approx -24dB)
    I2C_Write_LL(0x20, 0xD0);
    I2C_Write_LL(0x21, 0xD0);

    // Power Up
    I2C_Write_LL(0x02, 0x9E);

    return 0;
}
