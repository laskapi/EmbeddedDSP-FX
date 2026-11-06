#ifndef EMBEDDEDDSP_FIRMWARE_I2S_DMA_H
#define EMBEDDEDDSP_FIRMWARE_I2S_DMA_H

#include "main.h"
#include "Protocol/ProtocolCommon.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_spi.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace App::Audio {

/// @brief Owns I2S RX/TX circular DMA buffers and ready-half signalling.
///
/// One DMA half holds Protocol::AUDIO_SAMPLES stereo frames (L/R interleaved int16),
/// so one half maps 1:1 to one USB audio frame.
class I2sDma {
public:
    static constexpr std::size_t kStereoFramesPerHalf = Protocol::AUDIO_SAMPLES;
    static constexpr std::size_t kHalfSize = kStereoFramesPerHalf * 2;
    static constexpr std::size_t kBufSize = kHalfSize * 2;

    enum class ReadyHalf : std::int8_t { None, First, Second };

    /// @brief RX/TX pointers for one ping-pong half ready for CPU processing.
    struct DmaBuffers {
        const std::int16_t* rx{nullptr};
        std::int16_t* tx{nullptr};
    };

    /// @brief Arms RX/TX DMA circular transfers and enables I2S peripherals.
    void start() noexcept
    {
        const uint32_t txRegAddr = reinterpret_cast<uint32_t>(&(SPI3->DR));
        const uint32_t rxRegAddr = reinterpret_cast<uint32_t>(&(I2S3ext->DR));

        LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_0, rxRegAddr,
                               reinterpret_cast<uint32_t>(m_rx.data()),
                               LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, static_cast<uint32_t>(kBufSize));

        LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_5, reinterpret_cast<uint32_t>(m_tx.data()),
                               txRegAddr, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, static_cast<uint32_t>(kBufSize));

        LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_0);
        LL_DMA_EnableIT_HT(DMA1, LL_DMA_STREAM_0);

        LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
        LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);

        LL_SPI_EnableDMAReq_RX(I2S3ext);
        LL_SPI_EnableDMAReq_TX(SPI3);

        if (!LL_I2S_IsEnabled(SPI3)) {
            LL_I2S_Enable(SPI3);
        }
        if (!LL_I2S_IsEnabled(I2S3ext)) {
            LL_I2S_Enable(I2S3ext);
        }
    }

    void onHalfComplete() noexcept
    {
        m_ready.store(ReadyHalf::First, std::memory_order_relaxed);
    }

    void onTransferComplete() noexcept
    {
        m_ready.store(ReadyHalf::Second, std::memory_order_relaxed);
    }

    /// @brief Gives RX/TX buffers for the ready half (nullopt if idle).
    [[nodiscard]] std::optional<DmaBuffers> takeReadyBuffers() noexcept
    {
        const ReadyHalf half = m_ready.exchange(ReadyHalf::None, std::memory_order_relaxed);
        if (half == ReadyHalf::None) {
            return std::nullopt;
        }

        const std::size_t offset = (half == ReadyHalf::First) ? 0 : kHalfSize;
        return DmaBuffers{m_rx.data() + offset, m_tx.data() + offset};
    }

private:
    alignas(4) std::array<std::int16_t, kBufSize> m_rx{};
    alignas(4) std::array<std::int16_t, kBufSize> m_tx{};
    std::atomic<ReadyHalf> m_ready{ReadyHalf::None};
};

} // namespace App::Audio

#endif // EMBEDDEDDSP_FIRMWARE_I2S_DMA_H
