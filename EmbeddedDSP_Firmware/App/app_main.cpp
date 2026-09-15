#include "app_main.h"
#include "main.h"
#include <DSP/DynamicAudioPipeline.h>
#include <DSP/DelayEffect.h>
#include <DSP/OverdriveEffect.h>
#include <DSP/VirtualAudioSource.h>
#include <Protocol/ControlParser.h>
#include <Protocol/AudioFramePacket.h>
#include <Protocol/SpscQueue.h>

#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_bus.h"

#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <optional>

#define APP_USE_VIRTUAL_AUDIO_SOURCE 1

extern "C" uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

namespace
{
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

    alignas(4) std::array<std::int16_t, DMA_BUF_SIZE> dmaRxBuffer{};
    alignas(4) std::array<std::int16_t, DMA_BUF_SIZE> dmaTxBuffer{};

    DynamicAudioPipeline audioPipeline;
    std::atomic<BufferState> activeBufferState{BufferState::None};
    ControlParser protocolParser{audioPipeline};

#if APP_USE_VIRTUAL_AUDIO_SOURCE
    VirtualAudioSource* virtualSource = nullptr;
#endif

    using AudioTxQueue = Protocol::SpscQueue<Protocol::AudioFramePacket, 8>;
    AudioTxQueue audioTxQueue{};

    std::array<int16_t, Protocol::AUDIO_SAMPLES> txSampleAccumulator{};
    std::size_t txSampleCount = 0;
    uint8_t audioSequenceNumber = 0;

    [[nodiscard]] inline std::uint32_t floatToQ15SaturateStereo(float left, float right) noexcept
    {
        const auto left32 = static_cast<std::int32_t>(std::lroundf(left * AUDIO_SCALE_FACTOR));
        const auto right32 = static_cast<std::int32_t>(std::lroundf(right * AUDIO_SCALE_FACTOR));
        
        std::uint32_t packedInput{0};
        asm volatile("pkhbt %0, %1, %2, lsl #16" : "=r"(packedInput) : "r"(left32), "r"(right32));
        
        std::uint32_t packedResult{0};
        asm volatile("ssat16 %0, #16, %1" : "=r"(packedResult) : "r"(packedInput));
        
        return packedResult;
    }

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

            audioPipeline.process(L, R);
            txPtr32[i] = floatToQ15SaturateStereo(L * 0.8f, R * 0.8f);

            if (txSampleCount < Protocol::AUDIO_SAMPLES)
            {
                txSampleAccumulator[txSampleCount++] = static_cast<int16_t>(txPtr32[i] & 0xFFFF);
            }
        }

        if (txSampleCount >= Protocol::AUDIO_SAMPLES)
        {
            Protocol::AudioFramePacket pkt;
            pkt.sequenceNumber = audioSequenceNumber++;
            pkt.samples = txSampleAccumulator;
            pkt.applyCRC();

            audioTxQueue.push(pkt);
            txSampleCount = 0;
        }
    }

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
}

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

void app_main(I2S_HandleTypeDef* audio_i2s)
{
    audioPipeline.prepare(SYSTEM_SAMPLE_RATE);

#if APP_USE_VIRTUAL_AUDIO_SOURCE
    static VirtualAudioSource sourceInstance(SYSTEM_SAMPLE_RATE);
    virtualSource = &sourceInstance;
    virtualSource->setFrequency(440.0f);
    virtualSource->setWaveform(VirtualAudioSource::Waveform::Saw);
#endif

    start_i2s_dma_ll();

    uint32_t lastTick = HAL_GetTick();
    
    std::optional<Protocol::ControlPacket> pendingControl;
    std::optional<Protocol::AudioFramePacket> pendingAudio;

    while (1)
    {
        protocolParser.processRxQueue();

        if (!pendingControl.has_value())
        {
            pendingControl = protocolParser.getTxQueue().pop();
        }

        if (pendingControl.has_value())
        {
            if (CDC_Transmit_FS(reinterpret_cast<uint8_t*>(&(*pendingControl)), 
                                sizeof(Protocol::ControlPacket)) == 0)
            {
                pendingControl.reset();
            }
        }

        if (!pendingControl.has_value() && !pendingAudio.has_value())
        {
            pendingAudio = audioTxQueue.pop();
        }

        if (pendingAudio.has_value())
        {
            if (CDC_Transmit_FS(reinterpret_cast<uint8_t*>(&(*pendingAudio)), 
                                sizeof(Protocol::AudioFramePacket)) == 0)
            {
                pendingAudio.reset();
            }
        }

        BufferState stateToProcess = activeBufferState.exchange(BufferState::None, std::memory_order_relaxed);
        if (stateToProcess == BufferState::HalfReady) process_buffer_half(0);
        else if (stateToProcess == BufferState::FullReady) process_buffer_half(HALF_BUF_SIZE);

        if (HAL_GetTick() - lastTick >= 200)
        {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
            lastTick = HAL_GetTick();
        }
    }
}
}
