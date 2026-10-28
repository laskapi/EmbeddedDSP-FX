#ifndef EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H
#define EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H

#include "DSP/EffectChain.h"
#include "Protocol/ControlPacket.h"
#include "Protocol/SpscQueue.h"
#include <array>
#include <cstring>
#include <string_view>

#ifndef FIRMWARE_UNIT_TESTS
#include "usbd_cdc_if.h"
#else
/// @brief Unit-test stub for USB CDC transmit.
inline uint8_t CDC_Transmit_FS(uint8_t*, uint16_t) { return 0; }
#endif

/// @brief Parses USB control packets and reports EffectChain state to the host
class ControlParser {
public:
    static constexpr std::size_t RxBufferSize = 256;
    using TxQueue = Protocol::SpscQueue<Protocol::ControlPacket, 64>;

    explicit ControlParser(EffectChain& chain) : m_chain(chain) {}

    /// @brief Drains the Rx byte queue into the frame parser
    void processRxQueue() {
        while (auto b = m_rxQueue.pop()) parseByte(*b);
    }

    /// @brief Enqueues raw CDC bytes
    void onBytesReceived(const uint8_t* d, std::size_t l) {
        for (size_t i = 0; i < l; ++i) m_rxQueue.push(d[i]);
    }

    TxQueue& txQueue() { return m_txQueue; }

    /// @brief Forwards sample-rate updates to the effect chain
    void setSampleRate(float sr) { m_chain.updateSampleRate(sr); }

private:
    void parseByte(uint8_t byte) {
        if (m_rxIndex == 0 && byte != Protocol::SOF::Control) return;

        m_frameBuffer[m_rxIndex++] = byte;

        if (m_rxIndex == sizeof(Protocol::ControlPacket)) {
            Protocol::ControlPacket pkt;
            std::memcpy(&pkt, m_frameBuffer.data(), sizeof(Protocol::ControlPacket));

            if (pkt.isValid()) {
                applyPacket(pkt);
            }
            m_rxIndex = 0;
        }
    }

    /// @brief Applies a validated control packet to the effect chain.
    void applyPacket(const Protocol::ControlPacket& packet) noexcept {
        const uint8_t slotId = packet.slotId;
        switch (packet.command) {
            case Protocol::Command::SetParam:
                if (slotId < EffectChain::MAX_EFFECT_SLOTS) {
                    std::visit([id = packet.paramId, val = packet.value()](auto& fx) {
                        fx.setParamValue(id, val);
                    }, m_chain.slot(slotId));
                }
                break;

            case Protocol::Command::SetEffectType:
                m_chain.setEffectByIndex(slotId, packet.paramId);
                break;

            case Protocol::Command::BypassToggle:
                if (slotId < EffectChain::MAX_EFFECT_SLOTS) {
                    std::visit([](auto& fx) { fx.toggleBypass(); }, m_chain.slot(slotId));
                }
                break;

            case Protocol::Command::ClearSlot:
                m_chain.clearSlot(slotId);
                break;

            case Protocol::Command::SwapSlots:
                m_chain.swapSlots(slotId, packet.paramId);
                break;

            case Protocol::Command::SetActiveSlots:
                // v1: fixed chain size; EmptyEffect marks unused slots. Ignore.
                break;

            case Protocol::Command::GetState:
                handleGetStateRequest();
                break;

            default:
                break;
        }
    }

    /// @brief Sends discovery manifest, then full parameter state over CDC.
    void handleGetStateRequest() noexcept {
        Protocol::ControlPacket pkt;
        pkt.command = Protocol::Command::ReportState;
        pkt.signalId = Protocol::ReservedParam::ManifestSignal;
        pkt.applyCRC();

        while (CDC_Transmit_FS(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt)) != 0) {}

        auto manifest = m_chain.generateGlobalManifest();
        uint16_t len = static_cast<uint16_t>(manifest.size()) + 1;
        while (CDC_Transmit_FS(reinterpret_cast<uint8_t*>(const_cast<char*>(manifest.data())), len) != 0) {}

        reportFullState();
    }

    /// @brief Enqueues one host-bound report packet (blocks if Tx queue is full).
    void sendReport(Protocol::Command cmd, uint8_t slot, uint8_t paramOrType, float val) noexcept {
        Protocol::ControlPacket pkt;
        pkt.command = cmd;
        pkt.slotId = slot;
        pkt.paramId = paramOrType;
        pkt.setValue(val);
        pkt.applyCRC();
        while (!m_txQueue.push(pkt)) {}
    }

    /// @brief Reports effect type, bypass, and params for every slot.
    void reportFullState() noexcept {
        for (uint8_t slotId = 0; slotId < EffectChain::MAX_EFFECT_SLOTS; ++slotId) {
            std::visit([this, slotId](auto& fx) {
                using T = std::decay_t<decltype(fx)>;
                sendReport(Protocol::Command::SetEffectType, slotId, static_cast<uint8_t>(T::Id), 0.0f);
                sendReport(Protocol::Command::BypassToggle, slotId, 0, fx.isBypassed() ? 1.0f : 0.0f);
                for (uint8_t p = 0; p < T::ParamCount; ++p) {
                    sendReport(Protocol::Command::SetParam, slotId, p, fx.paramValue(p));
                }
            }, m_chain.slot(slotId));
        }
    }

    Protocol::SpscQueue<uint8_t, RxBufferSize> m_rxQueue{};
    TxQueue m_txQueue{};
    std::array<uint8_t, sizeof(Protocol::ControlPacket)> m_frameBuffer{};
    std::size_t m_rxIndex{0};
    EffectChain& m_chain;
};

#endif // EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H
