#include "app_main.h"
#include "DynamicAudioPipeline.h"
#include "DelayEffect.h"

#include <algorithm>
#include <array>
#include <atomic>

namespace {
constexpr std::size_t DMA_BUF_SIZE = 512;
constexpr std::size_t HALF_BUF_SIZE = DMA_BUF_SIZE / 2;
constexpr float AUDIO_SCALE_FACTOR = 32768.0f;

enum class BufferState : int8_t {
    None,
    HalfReady,
    FullReady
};

// Align buffers to 4-byte boundaries for hardware DMA transfer compatibility
alignas(4) std::array<std::int16_t, DMA_BUF_SIZE> dmaRxBuffer{};
alignas(4) std::array<std::int16_t, DMA_BUF_SIZE> dmaTxBuffer{};

DynamicAudioPipeline<2> audioPipeline;
std::atomic<BufferState> activeBufferState{BufferState::None};

/**
 * @brief Processes half of the interleaved stereo DMA buffer using float conversion and DSP pipeline.
 * @param offset Starting index in the DMA buffer (0 for first half, HALF_BUF_SIZE for second half).
 */
void process_buffer_half(std::size_t offset) {
    for (std::size_t i = 0; i < HALF_BUF_SIZE; i += 2) {
        std::size_t idx = offset + i;

        // Convert Q15 PCM to normalized float range [-1.0f, 1.0f]
        float leftSample = static_cast<float>(dmaRxBuffer[idx]) / AUDIO_SCALE_FACTOR;
        float rightSample = static_cast<float>(dmaRxBuffer[idx + 1]) / AUDIO_SCALE_FACTOR;

        audioPipeline.process(leftSample, rightSample);

        // Convert normalized float back to Q15 PCM with hard-clamping to prevent wrap-around distortion
        float clampedLeft = std::clamp(leftSample * AUDIO_SCALE_FACTOR, -AUDIO_SCALE_FACTOR, AUDIO_SCALE_FACTOR - 1.0f);
        float clampedRight = std::clamp(rightSample * AUDIO_SCALE_FACTOR, -AUDIO_SCALE_FACTOR, AUDIO_SCALE_FACTOR - 1.0f);

        dmaTxBuffer[idx] = static_cast<std::int16_t>(clampedLeft);
        dmaTxBuffer[idx + 1] = static_cast<std::int16_t>(clampedRight);
    }
}
} // namespace

extern "C" {

void app_main(I2S_HandleTypeDef* audio_i2s) {
    // Construct effect directly in slot 0 to avoid stack duplication
    audioPipeline.setEffectInSlot(0, DelayEffect{});

    if (auto* delay = std::get_if<DelayEffect>(&audioPipeline.getSlot(0))) {
        delay->setDelayTime(0.35f); // 350 ms
        delay->setFeedback(0.4f);
        delay->setDryWet(0.5f);
    }

    HAL_I2SEx_TransmitReceive_DMA(
        audio_i2s,
        reinterpret_cast<uint16_t*>(dmaTxBuffer.data()),
        reinterpret_cast<uint16_t*>(dmaRxBuffer.data()),
        DMA_BUF_SIZE
    );

    // Event loop processing DMA audio buffers outside ISR context
    while (1) {
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
            // Sleep CPU until the next DMA interrupt triggers
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
    // User Button (B1) debounce and bypass toggle
    if (GPIO_Pin == GPIO_PIN_13) {
        static uint32_t lastInterruptTime = 0;
        uint32_t currentTime = HAL_GetTick();

        if (currentTime - lastInterruptTime > 200) {
            if (auto* delay = std::get_if<DelayEffect>(&audioPipeline.getSlot(0))) {
                delay->toggleBypass();
            }
            lastInterruptTime = currentTime;
        }
    }
}

} // extern "C"