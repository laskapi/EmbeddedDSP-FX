#ifndef EMBEDDEDDSP_HOST_IDEVICEBACKEND_H
#ifndef EMBEDDEDDSP_HOST_IDEVICEBACKEND_H
#define EMBEDDEDDSP_HOST_IDEVICEBACKEND_H

#include <QObject>
#include <QString>
#include <vector>
#include "EffectSpec.h"

namespace Protocol {
    struct ControlPacket;
    struct AudioFramePacket;
}

namespace Host {

/// @brief Information about a potential connection.
struct ConnectionInfo {
    QString id;
    QString description;
};

/// @brief Interface for device communication backends (Serial, Simulator, etc.).
class IDeviceBackend : public QObject {
    Q_OBJECT
public:
    explicit IDeviceBackend(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IDeviceBackend() override = default;

    virtual bool connect(const QString &target) = 0;
    virtual void disconnect() = 0;
    virtual bool sendControlPacket(const Protocol::ControlPacket &pkt) = 0;
    virtual bool isOpen() const = 0;

    /// @return List of available connections for this backend.
    virtual std::vector<ConnectionInfo> enumerateConnections() const = 0;

signals:
    void manifestReady(const Host::DeviceManifest &manifest);
    void audioFrameReady(const Protocol::AudioFramePacket &frame);
    void controlPacketReady(const Protocol::ControlPacket &pkt);
    void connectionStatusChanged(bool connected, const QString &portName);
    void errorOccurred(const QString &errorMessage);
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_IDEVICEBACKEND_H
#endif
