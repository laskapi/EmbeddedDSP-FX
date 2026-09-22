#ifndef EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H
#define EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H

#include <DSP/DynamicAudioPipeline.h>
#include <Protocol/ControlPacket.h>
#include <Protocol/SpscQueue.h>
#include <usbd_cdc_if.h>
#include <cstring>
#include <string_view>

/// @brief Logic for dispatching USB control commands to the DSP pipeline.
class ControlParser {
public:
    static constexpr std::size_t RxBufferSize = 256;
    using TxQueue = Protocol::SpscQueue<Protocol::ControlPacket, 64>;

private:
    Protocol::SpscQueue<uint8_t, RxBufferSize> m_rxQueue{};
    TxQueue m_txQueue{};
    std::array<uint8_t, sizeof(Protocol::ControlPacket)> m_frameBuffer{};
    std::size_t m_rxIndex{0};
    DynamicAudioPipeline& m_pipeline;
    float m_sampleRate{48000.0f};

    void applyPacket(const Protocol::ControlPacket& pkt) noexcept;
    void handleGetStateRequest() noexcept;
    void sendReport(Protocol::Command cmd, uint8_t slot, uint8_t paramOrType, float val) noexcept;
    void reportFullState() noexcept;
    void reportSlotState(uint8_t slotId) noexcept;

public:
    explicit ControlParser(DynamicAudioPipeline& p);
    
    /// @brief Processes pending bytes from the internal RX queue.
    void processRxQueue();
    
    /// @brief Ingests raw bytes from the USB CDC driver.
    void onBytesReceived(const uint8_t* d, std::size_t l);
    
    TxQueue& getTxQueue() { return m_txQueue; }
    void setSampleRate(float sr) { m_sampleRate = sr; }

private:
    void parseByte(uint8_t byte);
};

#endif // EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H
