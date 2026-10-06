#ifndef EMBEDDEDDSP_HOST_APPCONTROLLER_H
#define EMBEDDEDDSP_HOST_APPCONTROLLER_H

#include <QObject>
#include <QMap>
#include <vector>
#include "EffectSpec.h"
#include "IDeviceBackend.h"

class QThread;

namespace Protocol {
    struct ControlPacket;
    struct AudioFramePacket;
}

namespace Host {

class AudioFrameWorker;

/// @brief Central controller managing UI-to-hardware communication hub.
class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController();
    ~AppController() override;

    void setParameter(uint8_t slotId, uint8_t paramId, float value);
    void setEffectType(uint8_t slotId, uint8_t effectTypeId);
    void setBypass(uint8_t slotId, bool bypassed);
    void requestSync();

    /// @return List of available connections based on current backend.
    std::vector<ConnectionInfo> availableConnections() const;

    void connectToDevice(const QString& target = QString());
    void disconnect();

signals:
    void manifestReady(const Host::DeviceManifest& manifest);
    void deviceStateUpdated(const Protocol::ControlPacket& pkt);
    void connectionChanged(bool connected, const QString& portName);
    void audioFrameReady(const Protocol::AudioFramePacket &frame);
    void errorOccurred(const QString &errorMessage);
    void spectrumReady(const std::vector<float>& magnitudeDb);

private slots:
    void onConnectionStatusChanged(bool connected, const QString& portName);

private:
    void setupBackend(IDeviceBackend* backend);
    void dispatchControlPacket(Protocol::ControlPacket pkt);

    QThread* m_workerThread{nullptr};
    AudioFrameWorker* m_audioWorker{nullptr};
    IDeviceBackend* m_backend{nullptr};
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_APPCONTROLLER_H
