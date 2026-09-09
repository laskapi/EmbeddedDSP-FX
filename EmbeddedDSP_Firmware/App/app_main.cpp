#include "app_main.h"
#include "main.h"
#include "DSP/DynamicAudioPipeline.h"
#include "DSP/DelayEffect.h"
#include "DSP/OverdriveEffect.h"
#include "DSP/VirtualAudioSource.h"
#include "Protocol/ControlParser.h"
#include "Protocol/AudioFramePacket.h"
#include "Protocol/Crc16Calculator.h"
#include "Protocol/SpscQueue.h"

#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_bus.h"

#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <optional>

#define APP_USE_VIRTUAL_AUDIO_SOURCE 0 //  0 - Switch to real I2S input!

extern "C" uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

namespace
{
    // DMA and Audio Buffer configuration
    constexpr std::size_t DMA_BUF_SIZE = 512;
    constexpr std::size_t HALF_BUF_SIZE = DMA_BUF_SIZE / 2;
    constexpr float AUDIO_SCALE_FACTOR = 32767.0f;
    constexpr float INV_AUDIO_SCALE_FACTOR = 1.0f / 32768.0f;
    constexpr float SYSTEM_SAMPLE_RATE = 48000.0f;

    enum class BufferState : int8_t
    {
        None,
        HalfReady,
        FullReady
    };

    // Peripheral buffers
    alignas(4) std::array<std::int16_t, DMA_BUF_SIZE> dmaRxBuffer{};
    alignas(4) std::array<std::int16_t, DMA_BUF_SIZE> dmaTxBuffer{};

    // DSP and Control components
    DynamicAudioPipeline audioPipeline;
    std::atomic<BufferState> activeBufferState{BufferState::None};
    ControlParser protocolParser{audioPipeline};

#if APP_USE_VIRTUAL_AUDIO_SOURCE
    VirtualAudioSource* virtualSource = nullptr;
#endif

    // Host telemetry communication
    using AudioTxQueue = SpscQueue<AudioFramePacket, 8>;
    AudioTxQueue audioTxQueue{};

    std::array<int16_t, AUDIO_PACKET_SAMPLES> txSampleAccumulator{};
    std::size_t txSampleCount = 0;
    uint8_t audioSequenceNumber = 0;

    /**
     * @brief Packs stereo float samples into a single 32-bit word with hardware saturation.
     * Uses ARM Cortex-M4 DSP instructions for maximum performance.
     */
    [[nodiscard]] inline std::uint32_t floatToQ15SaturateStereo(float left, float right) noexcept
    {
        const auto left32 = static_cast<std::int32_t>(std::lroundf(left * AUDIO_SCALE_FACTOR));
        const auto right32 = static_cast<std::int32_t>(std::lroundf(right * AUDIO_SCALE_FACTOR));
        
        std::uint32_t packedInput{0};
        // Pack two 16-bit values into one 32-bit register
        asm volatile("pkhbt %0, %1, %2, lsl #16" : "=r"(packedInput) : "r"(left32), "r"(right32));
        
        std::uint32_t packedResult{0};
        // Apply hardware saturation for both 16-bit halves simultaneously
        asm volatile("ssat16 %0, #16, %1" : "=r"(packedResult) : "r"(packedInput));
        
        return packedResult;
    }

    /**
     * @brief Processes one half of the DMA buffer.
     * Core loop for audio generation, DSP effects, and telemetry preparation.
     */
    void process_buffer_half(std::size_t offset)
    {
        uint32_t* txPtr32 = reinterpret_cast<uint32_t*>(&dmaTxBuffer[offset]);
        constexpr std::size_t SAMPLES_COUNT = HALF_BUF_SIZE / 2;

#if !APP_USE_VIRTUAL_AUDIO_SOURCE
        const int16_t* rxPtr = &dmaRxBuffer[offset];
#endif

        for (std::size_t i = 0; i < SAMPLES_COUNT; ++i)
        {
            float L, R;

#if APP_USE_VIRTUAL_AUDIO_SOURCE
            if (virtualSource) {
                auto sample = virtualSource->nextSample();
                L = sample.left;
                R = sample.right;
            } else {
                L = R = 0.0f;
            }
#else
            L = static_cast<float>(rxPtr[i * 2]) * INV_AUDIO_SCALE_FACTOR;
            R = static_cast<float>(rxPtr[i * 2 + 1]) * INV_AUDIO_SCALE_FACTOR;
#endif

            // Apply audio effects
            audioPipeline.process(L, R);

            // Convert to PCM16 with saturation and 0.8 gain headroom
            txPtr32[i] = floatToQ15SaturateStereo(L * 0.8f, R * 0.8f);

            // Prepare spectrum data for host
            if (txSampleCount < AUDIO_PACKET_SAMPLES)
            {
                txSampleAccumulator[txSampleCount++] = static_cast<int16_t>(txPtr32[i] & 0xFFFF);
            }
        }

        // Send telemetry packet to host if accumulator is full
        if (txSampleCount >= AUDIO_PACKET_SAMPLES)
        {
            AudioFramePacket pkt;
            pkt.sof = 0xA6;
            pkt.sequenceNumber = audioSequenceNumber++;
            pkt.payloadLength = AUDIO_PACKET_SAMPLES * sizeof(int16_t);

            for (size_t s = 0; s < AUDIO_PACKET_SAMPLES; ++s)
            {
                pkt.samples[s] = txSampleAccumulator[s];
            }

            const uint8_t* rawData = reinterpret_cast<const uint8_t*>(&pkt);
            pkt.crc16 = Crc16Calculator::calculate(rawData, sizeof(AudioFramePacket) - 2);

            audioTxQueue.push(pkt);
            txSampleCount = 0;
        }
    }

    /**
     * @brief Initializes I2S DMA transfers using Low-Layer (LL) drivers.
     */
    void start_i2s_dma_ll()
    {
        const uint32_t txRegAddr = (uint32_t)&(SPI3->DR);
        const uint32_t rxRegAddr = (uint32_t)&(I2S3ext->DR);

        LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_0, rxRegAddr, (uint32_t)dmaRxBuffer.data(),
                               LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, DMA_BUF_SIZE);

        LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_5, (uint32_t)dmaTxBuffer.data(), txRegAddr,
                               LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, DMA_BUF_SIZE);

        LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_0);
        LL_DMA_EnableIT_HT(DMA1, LL_DMA_STREAM_0);

        LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
        LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);

        LL_SPI_EnableDMAReq_RX(I2S3ext);
        LL_SPI_EnableDMAReq_TX(SPI3);

        if (!LL_I2S_IsEnabled(SPI3)) LL_I2S_Enable(SPI3);
        if (!LL_I2S_IsEnabled(I2S3ext)) LL_I2S_Enable(I2S3ext);
    }
} // namespace

extern "C" {
void ProtocolParser_OnBytesReceived(const uint8_t* Buf, uint32_t Len)
{
    protocolParser.onBytesReceived(Buf, Len);
}

void app_audio_half_transfer_cb(void)
{
    activeBufferState.store(BufferState::HalfReady, std::memory_order_relaxed);
}

void app_audio_transfer_complete_cb(void)
{
    activeBufferState.store(BufferState::FullReady, std::memory_order_relaxed);
}

/**
 * @brief Application entry point for firmware logic.
 */
void app_main(I2S_HandleTypeDef* audio_i2s)
{
    audioPipeline.prepare(SYSTEM_SAMPLE_RATE);

#if APP_USE_VIRTUAL_AUDIO_SOURCE
    static VirtualAudioSource sourceInstance(SYSTEM_SAMPLE_RATE);
    virtualSource = &sourceInstance;
    virtualSource->setFrequency(440.0f);
    virtualSource->setWaveform(VirtualAudioSource::Waveform::Saw); // Sawtooth for better overdrive testing
#endif

    start_i2s_dma_ll();

    uint32_t lastTick = HAL_GetTick();
    std::optional<AudioFramePacket> pendingPacket;

    while (1)
    {
        // Handle incoming control packets from PC
        protocolParser.processRxQueue();

        // Manage audio telemetry stream to PC
        if (!pendingPacket.has_value())
        {
            pendingPacket = audioTxQueue.pop();
        }

        if (pendingPacket.has_value())
        {
            if (CDC_Transmit_FS(reinterpret_cast<uint8_t*>(&(*pendingPacket)), sizeof(AudioFramePacket)) == 0)
            {
                pendingPacket.reset();
            }
        }

        // Trigger DSP processing when DMA buffer is ready
        BufferState stateToProcess = activeBufferState.exchange(BufferState::None, std::memory_order_relaxed);
        if (stateToProcess == BufferState::HalfReady) process_buffer_half(0);
        else if (stateToProcess == BufferState::FullReady) process_buffer_half(HALF_BUF_SIZE);

        // System status heartbeat
        if (HAL_GetTick() - lastTick >= 200)
        {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
            lastTick = HAL_GetTick();
        }

        // Power saving: wait for next event if no tasks are pending
        if (stateToProcess == BufferState::None && !pendingPacket.has_value())
        {
            __WFI();
        }
    }
}
} // extern "C"