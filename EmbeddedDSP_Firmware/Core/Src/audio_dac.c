#include "audio_dac.h"
#include "main.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_utils.h"

#define CS43L22_I2C_ADDR 0x94


#define WM8960_I2C_ADDR 0x34

/**
 * @brief Helper for WM8960 16-bit register writes (7-bit reg, 9-bit data)
 */
static int WM8960_Write(uint8_t reg, uint16_t value) {
    uint8_t b1 = (reg << 1) | ((value >> 8) & 0x01);
    uint8_t b2 = value & 0xFF;

    // Using the same LL logic but for WM8960 address
    uint32_t timeout = 10000;
    while (LL_I2C_IsActiveFlag_BUSY(I2C1) && --timeout);
    if (timeout == 0) return -1;

    LL_I2C_GenerateStartCondition(I2C1);
    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_SB(I2C1) && --timeout);

    LL_I2C_TransmitData8(I2C1, WM8960_I2C_ADDR);
    timeout = 10000;
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1) && --timeout);
    LL_I2C_ClearFlag_ADDR(I2C1);

    while (!LL_I2C_IsActiveFlag_TXE(I2C1));
    LL_I2C_TransmitData8(I2C1, b1);
    while (!LL_I2C_IsActiveFlag_TXE(I2C1));
    LL_I2C_TransmitData8(I2C1, b2);

    while (!LL_I2C_IsActiveFlag_BTF(I2C1));
    LL_I2C_GenerateStopCondition(I2C1);
    return 0;
}

int WM8960_Init(void) {
    // 1. Reset device
    if (WM8960_Write(0x0F, 0x000) != 0) return -1;
    HAL_Delay(10);

    // 2. Power Management: Enable VREF, VMID, ADC, DAC
    WM8960_Write(0x19, 0x0FC); // VMID=50k, VREF, AINL, AINR, ADCL, ADCR
    WM8960_Write(0x1A, 0x1F8); // DACL, DACR, LOUT1, ROUT1
    WM8960_Write(0x1B, 0x00C); // LINPUT1, RINPUT1 enabled

    // 3. Audio Interface: 0x01 = Left Justified (to match STM32), 16-bit
    WM8960_Write(0x07, 0x001);

    // 4. ADC Path: Connect LINPUT1/RINPUT1 to PGA
    WM8960_Write(0x20, 0x100); // ADCL Mux to PGA
    WM8960_Write(0x21, 0x100); // ADCR Mux to PGA
    WM8960_Write(0x2B, 0x001); // Input Mixer: LINPUT1 to PGA

    // 5. Input Gain: High gain for passive guitar (+30dB)
    // 0x13F: 0x100 (IPVU update) | 0x03F (+30dB)
    WM8960_Write(0x00, 0x13F);
    WM8960_Write(0x01, 0x13F);

    // 6. Digital Volume: Unity gain
    WM8960_Write(0x05, 0x000); // ADC Unmute

    return 0;
}


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

    // Interface Control: 0x00 = Left Justified (Verified working)
    I2C_Write_LL(0x06, 0x00);

    // Set master volume to a safe level (0xD0 approx -24dB)
    I2C_Write_LL(0x20, 0xD0);
    I2C_Write_LL(0x21, 0xD0);

    // Power Up
    I2C_Write_LL(0x02, 0x9E);

    return 0;
}