#ifndef EMBEDDEDDSP_HOST_IDEVICEBACKEND_H
#define EMBEDDEDDSP_HOST_IDEVICEBACKEND_H

#include <QObject>
#include <vector>
#include "ConnectionInfo.h"
#include "EffectSpec.h"

namespace Protocol {
    struct ControlPacket;
    struct AudioFramePacket;
}

namespace Host::Backend {

/// @brief Interface for device communication backends (Serial, Simulator, etc.).
class IDeviceBackend : public QObject {
    Q_OBJECT
public:
    explicit IDeviceBackend(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IDeviceBackend() override = default;

    virtual bool connectToDevice(const QString &target) = 0;
    virtual void disconnectFromDevice() = 0;
    virtual bool sendControlPacket(const Protocol::ControlPacket &pkt) = 0;
    virtual bool isOpen() const = 0;

    /// @return Selectable connection targets for this backend (ports, simulator, etc.).
    virtual std::vector<Host::Models::ConnectionInfo> availableConnections() const = 0;

signals:
    void manifestReady(const Host::Models::DeviceManifest &manifest);
    void audioFrameReady(const Protocol::AudioFramePacket &frame);
    void controlPacketReady(const Protocol::ControlPacket &pkt);
    void connectionStatusChanged(bool connected, const QString &portName);
    void errorOccurred(const QString &errorMessage);
};

} // namespace Host::Backend

#endif // EMBEDDEDDSP_HOST_IDEVICEBACKEND_H
