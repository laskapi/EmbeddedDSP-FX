#ifndef EMBEDDEDDSP_HOST_DEVICE_SESSION_H
#define EMBEDDEDDSP_HOST_DEVICE_SESSION_H

#include <QHash>
#include <QObject>
#include <QQueue>
#include <QString>
#include <cstdint>
#include <memory>
#include "Protocol/ControlPacket.h"

namespace Host::Models {
struct DeviceManifest;
}

namespace Protocol {
struct AudioFramePacket;
}

namespace Host::Device {

class IDevice;
class IDeviceFactory;

/// @brief Owns the active IDevice on the host worker thread; facade for open/close/send.
class DeviceSession : public QObject {
    Q_OBJECT
public:
    explicit DeviceSession(std::unique_ptr<IDeviceFactory> factory, QObject* parent = nullptr);
    ~DeviceSession() override;

    /// @brief Creates a device for @p connectionId, wires it, and opens it.
    void open(const QString& connectionId);

    /// @brief Closes the current device without destroying the session.
    void close();

    /// @brief Queues a control packet; SetParam coalesces by (slotId, paramId).
    bool sendControlPacket(const Protocol::ControlPacket& pkt);

signals:
    void manifestReady(const Host::Models::DeviceManifest& manifest);
    void audioFrameReady(const Protocol::AudioFramePacket& frame);
    void controlPacketReady(const Protocol::ControlPacket& pkt);
    void connectionStatusChanged(bool connected, const QString& portName);
    void errorOccurred(const QString& errorMessage);

private:
    void clearCurrentDevice();
    void clearPendingTx();
    void wireDevice(IDevice* device);
    void trySend();

    std::unique_ptr<IDeviceFactory> m_factory;
    std::unique_ptr<IDevice> m_device;

    /// @brief Latest-wins SetParam buffer; key = (slotId << 8) | paramId.
    QHash<uint16_t, Protocol::ControlPacket> m_pendingSetParams;
    QQueue<Protocol::ControlPacket> m_otherCommands;
    bool m_txBusy{false};
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_DEVICE_SESSION_H
