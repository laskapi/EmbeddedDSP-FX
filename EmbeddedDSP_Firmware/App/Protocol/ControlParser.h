#ifndef EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H
#define EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H

#include <DSP/DynamicAudioPipeline.h>
#include <Protocol/ControlPacket.h>
#include <Protocol/SpscQueue.h>
#include <usbd_cdc_if.h>
#include <cstring>
#include <string_view>

/**
 * @brief Logic for parsing incoming USB control packets and reporting state.
 */
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

    /** @brief Dispatches a valid packet to the pipeline. */
    void applyPacket(const Protocol::ControlPacket& pkt) noexcept;
    
    /** @brief Sends the discovery manifest and initial state reports. */
    void handleGetStateRequest() noexcept;

    /** @brief Helper to push a report packet to the Tx queue. */
    void sendReport(Protocol::Command cmd, uint8_t slot, uint8_t paramOrType, float val) noexcept;

    /** @brief Reports the state of all slots. */
    void reportFullState() noexcept;

    /** @brief Reports the state of a single slot. */
    void reportSlotState(uint8_t slotId) noexcept;

public:
    explicit ControlParser(DynamicAudioPipeline& p);
    
    /** @brief Processes all pending bytes in the Rx queue. */
    void processRxQueue();
    
    /** @brief Thread-safe ingestion of raw bytes from USB CDC. */
    void onBytesReceived(const uint8_t* d, std::size_t l);
    
    /** @return Reference to the transmission queue. */
    TxQueue& getTxQueue() { return m_txQueue; }
    
    /** @brief Updates the internal sample rate for effect initialization. */
    void setSampleRate(float sr) { m_sampleRate = sr; }

private:
    void parseByte(uint8_t byte);
};

#endif // EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H
