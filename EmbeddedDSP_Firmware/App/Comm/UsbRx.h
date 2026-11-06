#ifndef EMBEDDEDDSP_FIRMWARE_USB_RX_H
#define EMBEDDEDDSP_FIRMWARE_USB_RX_H

#include "Protocol/ControlPacket.h"
#include "SpscQueue.h"
#include <optional>

namespace App::Comm {

/// @brief Assembles ControlPackets from a raw USB CDC byte stream.
class UsbRx {
public:
    /// @brief Enqueues raw bytes (USB callback context).
    void onBytesReceived(const uint8_t* data, std::size_t length) {
        for (std::size_t i = 0; i < length; ++i) {
            m_rxQueue.push(data[i]);
        }
    }

    /// @brief Pulls bytes until one valid packet is assembled, or the Rx queue is empty.
    [[nodiscard]] std::optional<Protocol::ControlPacket> popPacket() {
        while (auto byte = m_rxQueue.pop()) {
            if (auto pkt = feed(*byte)) {
                return pkt;
            }
        }
        return std::nullopt;
    }

private:
    /// @brief Advances the packet assembler by one stream byte.
    [[nodiscard]] std::optional<Protocol::ControlPacket> feed(uint8_t byte) {
        if (m_rxIndex == 0 && byte != Protocol::SOF::Control) {
            return std::nullopt;
        }

        reinterpret_cast<uint8_t*>(&m_packet)[m_rxIndex++] = byte;

        if (m_rxIndex != sizeof(Protocol::ControlPacket)) {
            return std::nullopt;
        }

        m_rxIndex = 0;
        if (!m_packet.isValid()) {
            return std::nullopt;
        }
        return m_packet;
    }

    static constexpr std::size_t kRxByteCapacity = 128;

    App::SpscQueue<uint8_t, kRxByteCapacity> m_rxQueue{};
    Protocol::ControlPacket m_packet{};
    std::size_t m_rxIndex{0};
};

} // namespace App::Comm

#endif // EMBEDDEDDSP_FIRMWARE_USB_RX_H
