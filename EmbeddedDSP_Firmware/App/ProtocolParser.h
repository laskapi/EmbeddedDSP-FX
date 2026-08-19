#ifndef EMBEDDEDDSP_PROTOCOL_PARSER_H
#define EMBEDDEDDSP_PROTOCOL_PARSER_H

#include "Protocol.h"
#include "LockFreeQueue.h"
#include "DynamicAudioPipeline.h"
#include "OverdriveEffect.h"
#include "DelayEffect.h"
#include <array>
#include <cstring>
#include <variant>

/**
 * @brief Stateful parser processing incoming USB byte stream and dispatching
 *        control packets directly to DSP pipeline slots.
 * @tparam SlotsCount Number of audio effect slots in the dynamic pipeline.
 */

class ProtocolParser {
public:
    static constexpr size_t RxBufferSize = 256;

    explicit ProtocolParser(DynamicAudioPipeline& pipeline) noexcept
        : pipeline_{pipeline} {}

    /**
     * @brief Called by USB ISR/callback to push incoming raw bytes into the lock-free queue.
     * @param data Pointer to received byte buffer.
     * @param len Number of received bytes.
     */
    void onBytesReceived(const uint8_t* data, size_t len) noexcept {
        for (size_t i = 0; i < len; ++i) {
            rxQueue_.push(data[i]);
        }
    }

    /**
     * @brief Periodically called in the main background loop to process queued bytes.
     */
    void processRxQueue() noexcept {
        while (auto byteOpt = rxQueue_.pop()) {
            parseByte(*byteOpt);
        }
    }

private:
    DynamicAudioPipeline& pipeline_;
    LockFreeQueue<uint8_t, RxBufferSize> rxQueue_{};

    std::array<uint8_t, sizeof(Protocol::ControlPacket)> frameBuffer_{};
    size_t rxIndex_{0};

    void parseByte(uint8_t byte) noexcept {
        // Search for Start of Frame (SOF) byte (0xA5)
        if (rxIndex_ == 0) {
            if (byte == 0xA5) {
                frameBuffer_[0] = byte;
                rxIndex_ = 1;
            }
            return;
        }

        // Accumulate remaining packet bytes
        frameBuffer_[rxIndex_++] = byte;

        // Process frame once full length (9 bytes) is reached
        if (rxIndex_ == sizeof(Protocol::ControlPacket)) {
            Protocol::ControlPacket packet;
            std::memcpy(&packet, frameBuffer_.data(), sizeof(Protocol::ControlPacket));

            if (packet.isValid()) {
                applyPacket(packet);
            }

            rxIndex_ = 0; // Reset index for the next frame
        }
    }

    void applyPacket(const Protocol::ControlPacket& packet) noexcept {
        const float val = packet.getValue();

        if (packet.command == Protocol::Command::SetParam) {
            std::visit([paramId = packet.paramId, val](auto& effect) {
                using T = std::decay_t<decltype(effect)>;

                if constexpr (std::is_same_v<T, OverdriveEffect>) {
                    switch (paramId) {
                        case 0: effect.setDrive(val); break;
                        case 1: effect.setTone(val); break;
                        case 2: effect.setWet(val); break;
                        case 3: effect.setLevel(val); break;
                    }
                }
                else if constexpr (std::is_same_v<T, DelayEffect>) {
                    switch (paramId) {
                        case 0: effect.setDelayTime(val); break;
                        case 1: effect.setFeedback(val); break;
                        case 2: effect.setDryWet(val); break;
                    }
                }
            }, pipeline_.getSlot(packet.slotId));
        }
    }
};

#endif // EMBEDDEDDSP_PROTOCOL_PARSER_H