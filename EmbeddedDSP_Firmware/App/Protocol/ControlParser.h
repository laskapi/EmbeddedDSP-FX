#ifndef EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H
#define EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H

#include <DSP/DynamicAudioPipeline.h>
#include <Protocol/ControlPacket.h>
#include <Protocol/SpscQueue.h>
#include <usbd_cdc_if.h>
#include <cstring>
#include <string_view>

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

    void applyPacket(const Protocol::ControlPacket& pkt) noexcept {
        const uint8_t slotId = pkt.slotId;
        switch (pkt.command) {
            case Protocol::Command::SetParam:
                if (slotId < DynamicAudioPipeline::MAX_AUDIO_SLOTS) {
                    std::visit([id = pkt.paramId, val = pkt.getValue()](auto& fx) { 
                        fx.setParamValue(id, val); 
                    }, m_pipeline.getSlot(slotId));
                }
                break;

            case Protocol::Command::SetEffectType:
                m_pipeline.setEffectByIndex(slotId, pkt.effectTypeId, m_sampleRate);
                reportSlotState(slotId);
                break;

            case Protocol::Command::BypassToggle:
                if (slotId < DynamicAudioPipeline::MAX_AUDIO_SLOTS) {
                    std::visit([](auto& fx) { fx.toggleBypass(); }, m_pipeline.getSlot(slotId));
                    reportSlotState(slotId);
                }
                break;

            case Protocol::Command::ClearSlot:
                m_pipeline.clearSlot(slotId);
                reportSlotState(slotId);
                break;

            case Protocol::Command::SwapSlots:
                m_pipeline.swapSlots(slotId, pkt.targetSlotId);
                reportSlotState(slotId);
                reportSlotState(pkt.targetSlotId);
                break;

            case Protocol::Command::SetActiveSlots:
                m_pipeline.setActiveSlotsCount(slotId);
                break;

            case Protocol::Command::GetState:
                handleGetStateRequest();
                break;

            default: 
                break;
        }
    }

    void handleGetStateRequest() noexcept {
        Protocol::ControlPacket pkt;
        pkt.command = Protocol::Command::ReportState; 
        pkt.signalId = Protocol::ReservedParam::ManifestSignal; 
        pkt.applyCRC();
        
        while (CDC_Transmit_FS(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt)) != 0) {}

        auto manifest = m_pipeline.generateGlobalManifest();
        uint16_t len = static_cast<uint16_t>(manifest.size()) + 1;
        while (CDC_Transmit_FS(reinterpret_cast<uint8_t*>(const_cast<char*>(manifest.data())), len) != 0) {}

        reportFullState();
    }

    void sendReport(Protocol::Command cmd, uint8_t slot, uint8_t paramOrType, float val) noexcept {
        Protocol::ControlPacket pkt;
        pkt.command = cmd; 
        pkt.slotId = slot; 
        pkt.paramId = paramOrType; 
        pkt.setValue(val);
        pkt.applyCRC();
        while (!m_txQueue.push(pkt)) {}
    }

    void reportFullState() noexcept {
        for (uint8_t slotId = 0; slotId < DynamicAudioPipeline::MAX_AUDIO_SLOTS; ++slotId) {
            reportSlotState(slotId);
        }
    }

    void reportSlotState(uint8_t slotId) noexcept {
        if (slotId >= DynamicAudioPipeline::MAX_AUDIO_SLOTS) return;
        std::visit([this, slotId](auto& fx) {
            using T = std::decay_t<decltype(fx)>;
            sendReport(Protocol::Command::SetEffectType, slotId, static_cast<uint8_t>(T::Id), 0.0f);
            sendReport(Protocol::Command::BypassToggle, slotId, 0, fx.isBypassed() ? 1.0f : 0.0f);
            for (uint8_t p = 0; p < T::ParamCount; ++p) {
                sendReport(Protocol::Command::SetParam, slotId, p, fx.getParamValue(p));
            }
        }, m_pipeline.getSlot(slotId));
    }

public:
    explicit ControlParser(DynamicAudioPipeline& p) : m_pipeline(p) {}
    
    void processRxQueue() { 
        while (auto b = m_rxQueue.pop()) parseByte(*b); 
    }
    
    void onBytesReceived(const uint8_t* d, std::size_t l) { 
        for (size_t i=0; i<l; ++i) m_rxQueue.push(d[i]); 
    }
    
    TxQueue& getTxQueue() { return m_txQueue; }
    void setSampleRate(float sr) { m_sampleRate = sr; }

private:
    void parseByte(uint8_t byte) {
        if (m_rxIndex == 0 && byte != Protocol::SOF::Control) return;
        m_frameBuffer[m_rxIndex++] = byte;
        
        if (m_rxIndex == sizeof(Protocol::ControlPacket)) {
            Protocol::ControlPacket pkt;
            std::memcpy(&pkt, m_frameBuffer.data(), sizeof(pkt));
            if (pkt.isValid()) {
                applyPacket(pkt);
            }
            m_rxIndex = 0;
        }
    }
};

#endif // EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H
