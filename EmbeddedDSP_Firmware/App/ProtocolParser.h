#ifndef EMBEDDEDDSP_PROTOCOL_PARSER_H
#define EMBEDDEDDSP_PROTOCOL_PARSER_H

#include "DelayEffect.h"
#include "DynamicAudioPipeline.h"
#include "LockFreeQueue.h"
#include "OverdriveEffect.h"
#include "Protocol.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <variant>

/**
 * @brief Stateful parser processing incoming USB byte stream and dispatching
 *        control packets directly to DSP pipeline slots.
 */
class ProtocolParser {
public:
    static constexpr std::size_t RxBufferSize = 256;

private:
    LockFreeQueue<uint8_t, RxBufferSize> m_rxQueue{};
    std::array<uint8_t, sizeof(Protocol::ControlPacket)> m_frameBuffer{};
    std::size_t m_rxIndex{0};
    DynamicAudioPipeline& m_pipeline;

    void parseByte(uint8_t byte) noexcept {
        // Search for Start of Frame (SOF) byte (0xA5)
        if (m_rxIndex == 0) {
            if (byte == 0xA5) {
                m_frameBuffer[0] = byte;
                m_rxIndex = 1;
            }
            return;
        }

        // Accumulate remaining packet bytes
        m_frameBuffer[m_rxIndex++] = byte;

        // Process frame once full length (9 bytes) is reached
        if (m_rxIndex == sizeof(Protocol::ControlPacket)) {
            Protocol::ControlPacket packet;
            std::memcpy(&packet, m_frameBuffer.data(), sizeof(Protocol::ControlPacket));

            if (packet.isValid()) {
                applyPacket(packet);
            }

            m_rxIndex = 0; // Reset index for the next frame
        }
    }

    void applyPacket(const Protocol::ControlPacket& packet) noexcept {
        const float val = packet.getValue();
        const uint8_t slotId = packet.slotId;

        switch (packet.command) {
            case Protocol::Command::SetParam: {
                if (slotId < MAX_AUDIO_SLOTS) {
                    std::visit([paramId = packet.paramId, val](auto& effect) {
                        using T = std::decay_t<decltype(effect)>;

                        if constexpr (std::is_same_v<T, OverdriveEffect>) {
                            switch (paramId) {
                                case 0: effect.setDrive(val); break;
                                case 1: effect.setTone(val); break;
                                case 2: effect.setWet(val); break;
                                case 3: effect.setLevel(val); break;
                                default: break;
                            }
                        }
                        else if constexpr (std::is_same_v<T, DelayEffect>) {
                            switch (paramId) {
                                case 0: effect.setDelayTime(val); break;
                                case 1: effect.setFeedback(val); break;
                                case 2: effect.setDryWet(val); break;
                                default: break;
                            }
                        }
                    }, m_pipeline.getSlot(slotId));
                }
                break;
            }

            case Protocol::Command::SetEffectType: {
                if (slotId < MAX_AUDIO_SLOTS) {
                    const auto effectType = static_cast<uint8_t>(val);
                    switch (effectType) {
                        case 0: m_pipeline.clearSlot(slotId); break;
                        case 1: m_pipeline.setEffectInSlot(slotId, DelayEffect{}); break;
                        case 2: m_pipeline.setEffectInSlot(slotId, OverdriveEffect{}); break;
                        default: break;
                    }
                }
                break;
            }

            case Protocol::Command::BypassToggle: {
                if (slotId < MAX_AUDIO_SLOTS) {
                    std::visit([](auto& fx) {
                        fx.toggleBypass();
                    }, m_pipeline.getSlot(slotId));
                }
                break;
            }

            case Protocol::Command::ClearSlot: {
                m_pipeline.clearSlot(slotId);
                break;
            }

            case Protocol::Command::SwapSlots: {
                m_pipeline.swapSlots(slotId, packet.paramId);
                break;
            }

            case Protocol::Command::SetActiveSlots: {
                m_pipeline.setActiveSlotsCount(slotId);
                break;
            }
        }
    }

public:
    explicit ProtocolParser(DynamicAudioPipeline& pipeline) noexcept
        : m_pipeline{pipeline} {}

    /**
     * @brief Called by USB ISR/callback to push incoming raw bytes into the lock-free queue.
     * @param data Pointer to received byte buffer.
     * @param len Number of received bytes.
     */
    void onBytesReceived(const uint8_t* data, std::size_t len) noexcept {
        for (std::size_t i = 0; i < len; ++i) {
            m_rxQueue.push(data[i]);
        }
    }

    /**
     * @brief Periodically called in the main background loop to process queued bytes.
     */
    void processRxQueue() noexcept {
        while (auto byteOpt = m_rxQueue.pop()) {
            parseByte(*byteOpt);
        }
    }
};

#endif // EMBEDDEDDSP_PROTOCOL_PARSER_H