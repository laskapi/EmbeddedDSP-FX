#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include "Protocol.h"
#include "LockFreeQueue.h"
#include "DynamicAudioPipeline.h"
#include "OverdriveEffect.h"
#include "DelayEffect.h"
#include <array>
#include <cstring>
#include <variant>

template <size_t SlotsCount = 2>
class ProtocolParser {
public:
    static constexpr size_t RxBufferSize = 256;

    explicit ProtocolParser(DynamicAudioPipeline<SlotsCount>& pipeline) noexcept
        : pipeline_{pipeline} {}

    // Metoda wywoływana przez przerwanie USB
    void onBytesReceived(const uint8_t* data, size_t len) noexcept {
        for (size_t i = 0; i < len; ++i) {
            rxQueue_.push(data[i]);
        }
    }

    // Metoda wywoływana cyklicznie w pętli głównej (while(1))
    void processRxQueue() noexcept {
        while (auto byteOpt = rxQueue_.pop()) {
            parseByte(*byteOpt);
        }
    }

private:
    DynamicAudioPipeline<SlotsCount>& pipeline_;
    LockFreeQueue<uint8_t, RxBufferSize> rxQueue_{};

    std::array<uint8_t, sizeof(Protocol::ControlPacket)> frameBuffer_{};
    size_t rxIndex_{0};

    void parseByte(uint8_t byte) noexcept {
        // Szukanie bajtu startowego SOF (0xA5)
        if (rxIndex_ == 0) {
            if (byte == 0xA5) {
                frameBuffer_[0] = byte;
                rxIndex_ = 1;
            }
            return;
        }

        // Gromadzenie reszty ramki
        frameBuffer_[rxIndex_++] = byte;

        // Gdy odebrano pełną ramkę (9 bajtów)
        if (rxIndex_ == sizeof(Protocol::ControlPacket)) {
            Protocol::ControlPacket packet;
            std::memcpy(&packet, frameBuffer_.data(), sizeof(Protocol::ControlPacket));

            if (packet.isValid()) {
                applyPacket(packet);
            }

            rxIndex_ = 0; // Reset pod kolejną ramkę
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
                        case 1: effect.setTone(val); break; // Dopasowano do setTone w app_main
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

#endif // PROTOCOL_PARSER_H