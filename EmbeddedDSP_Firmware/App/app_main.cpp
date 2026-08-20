#include "app_main.h"
#include "DynamicAudioPipeline.h"
#include "DelayEffect.h"
#include "OverdriveEffect.h"
#include "ProtocolParser.h"

#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_spi.h"

#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>

namespace {
constexpr std::size_t DMA_BUF_SIZE = 512;
constexpr std::size_t HALF_BUF_SIZE = DMA_BUF_SIZE / 2;
constexpr float AUDIO_SCALE_FACTOR = 32768.0f;
constexpr float INV_AUDIO_SCALE_FACTOR = 1.0f / 32768.0f;
constexpr float SYSTEM_SAMPLE_RATE = 48000.0f;

enum class BufferState : int8_t {
    None,
    HalfReady,
    FullReady
};

// Align buffers to 4-byte boundaries for 32-bit DMA/SIMD access
alignas(4) std::array<std::int16_t, DMA_BUF_SIZE> dmaRxBuffer{};
alignas(4) std::array<std::int16_t, DMA_BUF_SIZE> dmaTxBuffer{};

DynamicAudioPipeline audioPipeline;
std::atomic<BufferState> activeBufferState{BufferState::None};

ProtocolParser protocolParser{audioPipeline};

// Converts float stereo pair (Left, Right) to packed 32-bit Q15 format [R:31..16 | L:15..0]
[[nodiscard]] inline std::uint32_t floatToQ15SaturateStereo(float left, float right) noexcept {
    const auto left32  = static_cast<std::int32_t>(std::lroundf(left  * AUDIO_SCALE_FACTOR));
    const auto right32 = static_cast<std::int32_t>(std::lroundf(right * AUDIO_SCALE_FACTOR));

    // Pack left (lower 16 bits) and right (upper 16 bits) into a single 32-bit register
    std::uint32_t packedInput{0};
    asm volatile("pkhbt %0, %1, %2, lsl #16"
                 : "=r"(packedInput)
                 : "r"(left32), "r"(right32));

    // Parallel SIMD saturation of both channels using Cortex-M4 SSAT16
    std::uint32_t packedResult{0};
    asm volatile("ssat16 %0, #16, %1"
                 : "=r"(packedResult)
                 : "r"(packedInput));

    return packedResult;
}

// Processes half of the DMA buffer using 32-bit SIMD access and float DSP
void process_buffer_half(std::size_t offset) {
    const auto* __restrict rxPtr32 = reinterpret_cast<const std::uint32_t*>(&dmaRxBuffer[offset]);
    auto*       __restrict txPtr32 = reinterpret_cast<std::uint32_t*>(&dmaTxBuffer[offset]);

    constexpr std::size_t STEREO_SAMPLES_COUNT = HALF_BUF_SIZE / 2;

    for (std::size_t i = 0; i < STEREO_SAMPLES_COUNT; ++i) {
        // Read packed stereo sample (32-bit)
        const std::uint32_t packedRx = rxPtr32[i];

        const auto leftInt  = static_cast<std::int16_t>(packedRx & 0xFFFF);
        const auto rightInt = static_cast<std::int16_t>(packedRx >> 16);

        float leftSample  = static_cast<float>(leftInt)  * INV_AUDIO_SCALE_FACTOR;
        float rightSample = static_cast<float>(rightInt) * INV_AUDIO_SCALE_FACTOR;

        audioPipeline.process(leftSample, rightSample);

        // Pack and saturate processed audio using SIMD
        txPtr32[i] = floatToQ15SaturateStereo(leftSample, rightSample);
    }
}

// Low-Layer DMA & I2S start routine
void start_i2s_dma_ll(SPI_TypeDef* i2sInstance, DMA_TypeDef* dmaInstance, uint32_t rxStream, uint32_t txStream) {
    const uint32_t dataRegAddr = LL_SPI_DMA_GetRegAddr(i2sInstance);

    LL_DMA_SetDataLength(dmaInstance, rxStream, DMA_BUF_SIZE);
    LL_DMA_SetDataLength(dmaInstance, txStream, DMA_BUF_SIZE);

    LL_DMA_ConfigAddresses(
        dmaInstance,
        rxStream,
        dataRegAddr,
        reinterpret_cast<uint32_t>(dmaRxBuffer.data()),
        LL_DMA_DIRECTION_PERIPH_TO_MEMORY
    );

    LL_DMA_ConfigAddresses(
        dmaInstance,
        txStream,
        reinterpret_cast<uint32_t>(dmaTxBuffer.data()),
        dataRegAddr,
        LL_DMA_DIRECTION_MEMORY_TO_PERIPH
    );

    LL_DMA_EnableStream(dmaInstance, rxStream);
    LL_DMA_EnableStream(dmaInstance, txStream);

    LL_I2S_EnableDMAReq_RX(i2sInstance);
    LL_I2S_EnableDMAReq_TX(i2sInstance);

    if (!LL_I2S_IsEnabled(i2sInstance)) {
        LL_I2S_Enable(i2sInstance);
    }
}

} // namespace

extern "C" {

void ProtocolParser_OnBytesReceived(const uint8_t* Buf, uint32_t Len) {
    protocolParser.onBytesReceived(Buf, Len);
}

void app_main(I2S_HandleTypeDef* audio_i2s) {
    // Setup Overdrive
    audioPipeline.setEffectInSlot(0, OverdriveEffect{});
    if (auto* overdrive = std::get_if<OverdriveEffect>(&audioPipeline.getSlot(0))) {
        overdrive->prepare(SYSTEM_SAMPLE_RATE);
        overdrive->setDrive(6.0f);
        overdrive->setTone(3500.0f);
        overdrive->setWet(1.0f);
        overdrive->setLevel(0.9f);
    }

    // Setup Delay
    audioPipeline.setEffectInSlot(1, DelayEffect{});
    if (auto* delay = std::get_if<DelayEffect>(&audioPipeline.getSlot(1))) {
        delay->prepare(SYSTEM_SAMPLE_RATE);
        delay->setDelayTime(0.35f);
        delay->setFeedback(0.4f);
        delay->setDryWet(0.4f);
    }

    if (audio_i2s != nullptr) {
        start_i2s_dma_ll(SPI2, DMA1, LL_DMA_STREAM_3, LL_DMA_STREAM_4);
    }

    // Processing loop
    while (1) {
        protocolParser.processRxQueue();

        BufferState stateToProcess = activeBufferState.exchange(BufferState::None, std::memory_order_relaxed);
        switch (stateToProcess) {
        case BufferState::HalfReady:
            process_buffer_half(0);
            break;
        case BufferState::FullReady:
            process_buffer_half(HALF_BUF_SIZE);
            break;
        case BufferState::None:
        default:
            __WFI();
            break;
        }
    }
}

void app_audio_half_transfer_cb(void) {
    activeBufferState.store(BufferState::HalfReady, std::memory_order_relaxed);
}

void app_audio_transfer_complete_cb(void) {
    activeBufferState.store(BufferState::FullReady, std::memory_order_relaxed);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_13) {
        static uint32_t lastInterruptTime = 0;
        uint32_t currentTime = HAL_GetTick();

        if (currentTime - lastInterruptTime > 200) {
            if (auto* overdrive = std::get_if<OverdriveEffect>(&audioPipeline.getSlot(0))) {
                overdrive->toggleBypass();
            }
            lastInterruptTime = currentTime;
        }
    }
}

} // extern "C"