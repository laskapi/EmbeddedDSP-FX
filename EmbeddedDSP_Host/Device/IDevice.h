#ifndef EMBEDDEDDSP_HOST_IDEVICE_H
#define EMBEDDEDDSP_HOST_IDEVICE_H

#include <QObject>
#include "DeviceModel.h"

namespace Protocol {
struct ControlPacket;
struct AudioFramePacket;
}

namespace Host::Device {

/// @brief Host-side port for talking to a real or simulated device.
class IDevice : public QObject {
    Q_OBJECT
public:
    explicit IDevice(QObject* parent = nullptr) : QObject(parent) {}
    ~IDevice() override = default;

    virtual bool open(const QString& connectionId) = 0;
    virtual void close() = 0;
    virtual bool sendControlPacket(const Protocol::ControlPacket& pkt) = 0;

signals:
    void manifestReady(const Host::Models::DeviceManifest& manifest);
    void audioFrameReady(const Protocol::AudioFramePacket& frame);
    void controlPacketReady(const Protocol::ControlPacket& pkt);
    void connectionStatusChanged(bool connected, const QString& portName);
    void errorOccurred(const QString& errorMessage);
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_IDEVICE_H
