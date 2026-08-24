#ifndef EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H
#define EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H

#include "../DSP/DelayEffect.h"
#include "../DSP/DynamicAudioPipeline.h"
#include "../DSP/OverdriveEffect.h"
#include "ControlPacket.h"
#include "EffectParams.h"
#include "SpscQueue.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <variant>

/**
 * @brief Stateful parser processing incoming USB control stream.
 *        Dispatches packets to DSP pipeline slots using EffectParams schema.
 */
class ControlParser {
public:
    static constexpr std::size_t RxBufferSize = 256;
    static constexpr float DefaultSampleRate = 48000.0f;

private:
    SpscQueue<uint8_t, RxBufferSize> m_rxQueue{};
    std::array<uint8_t, sizeof(ControlPacket::ControlPacket)> m_frameBuffer{};
    std::size_t m_rxIndex{0};
    DynamicAudioPipeline& m_pipeline;
    float m_sampleRate{DefaultSampleRate};

    void parseByte(uint8_t byte) noexcept {
        if (m_rxIndex == 0) {
            if (byte == 0xA5) { // SOF
                m_frameBuffer[0] = byte;
                m_rxIndex = 1;
            }
            return;
        }

        m_frameBuffer[m_rxIndex++] = byte;

        if (m_rxIndex == sizeof(ControlPacket::ControlPacket)) {
            ControlPacket::ControlPacket packet;
            std::memcpy(&packet, m_frameBuffer.data(), sizeof(ControlPacket::ControlPacket));

            if (packet.isValid()) {
                applyPacket(packet);
            }
            m_rxIndex = 0;
        }
    }

    void applyPacket(const ControlPacket::ControlPacket& packet) noexcept {
        const float val = packet.getValue();
        const uint8_t slotId = packet.slotId;

        switch (packet.command) {
            case ControlPacket::Command::SetParam: {
                if (slotId < MAX_AUDIO_SLOTS) {
                    std::visit([paramId = packet.paramId, val](auto& effect) {
                        using T = std::decay_t<decltype(effect)>;

                        if constexpr (std::is_same_v<T, OverdriveEffect>) {
                            switch (static_cast<EffectParams::OverdriveParam>(paramId)) {
                                case EffectParams::OverdriveParam::Drive: effect.setDrive(val); break;
                                case EffectParams::OverdriveParam::Tone:  effect.setTone(val); break;
                                case EffectParams::OverdriveParam::Wet:   effect.setWet(val); break;
                                case EffectParams::OverdriveParam::Level: effect.setLevel(val); break;
                                default: break;
                            }
                        }
                        else if constexpr (std::is_same_v<T, DelayEffect>) {
                            switch (static_cast<EffectParams::DelayParam>(paramId)) {
                                case EffectParams::DelayParam::Time:     effect.setDelayTime(val); break;
                                case EffectParams::DelayParam::Feedback: effect.setFeedback(val); break;
                                case EffectParams::DelayParam::DryWet:   effect.setDryWet(val); break;
                                default: break;
                            }
                        }
                    }, m_pipeline.getSlot(slotId));
                }
                break;
            }

            case ControlPacket::Command::SetEffectType: {
                if (slotId < MAX_AUDIO_SLOTS) {
                    const auto type = static_cast<EffectParams::EffectType>(packet.paramId);
                    switch (type) {
                        case EffectParams::EffectType::Empty:
                            m_pipeline.clearSlot(slotId);
                            break;
                        case EffectParams::EffectType::Delay: {
                            DelayEffect delay{};
                            delay.prepare(m_sampleRate);
                            m_pipeline.setEffectInSlot(slotId, std::move(delay));
                            break;
                        }
                        case EffectParams::EffectType::Overdrive: {
                            OverdriveEffect overdrive{};
                            overdrive.prepare(m_sampleRate);
                            m_pipeline.setEffectInSlot(slotId, std::move(overdrive));
                            break;
                        }
                    }
                }
                break;
            }

            case ControlPacket::Command::BypassToggle: {
                if (slotId < MAX_AUDIO_SLOTS) {
                    std::visit([](auto& fx) {
                        fx.toggleBypass();
                    }, m_pipeline.getSlot(slotId));
                }
                break;
            }

            case ControlPacket::Command::ClearSlot: {
                m_pipeline.clearSlot(slotId);
                break;
            }

            case ControlPacket::Command::SwapSlots: {
                m_pipeline.swapSlots(slotId, packet.paramId);
                break;
            }

            case ControlPacket::Command::SetActiveSlots: {
                m_pipeline.setActiveSlotsCount(slotId);
                break;
            }
        }
    }

public:
    explicit ControlParser(DynamicAudioPipeline& pipeline,
                           float sampleRate = DefaultSampleRate) noexcept
        : m_pipeline{pipeline}
        , m_sampleRate{sampleRate} {}

    void setSampleRate(float sampleRate) noexcept {
        m_sampleRate = (sampleRate > 0.0f) ? sampleRate : DefaultSampleRate;
    }

    [[nodiscard]] float getSampleRate() const noexcept {
        return m_sampleRate;
    }

    void onBytesReceived(const uint8_t* data, std::size_t len) noexcept {
        for (std::size_t i = 0; i < len; ++i) {
            m_rxQueue.push(data[i]);
        }
    }

    void processRxQueue() noexcept {
        while (auto byteOpt = m_rxQueue.pop()) {
            parseByte(*byteOpt);
        }
    }
};

#endif // EMBEDDEDDSP_FIRMWARE_CONTROL_PARSER_H