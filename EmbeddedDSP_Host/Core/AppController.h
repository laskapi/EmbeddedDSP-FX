#ifndef EMBEDDEDDSP_HOST_APPCONTROLLER_H
#define EMBEDDEDDSP_HOST_APPCONTROLLER_H

#include <QObject>
#include <QString>
#include <memory>
#include <vector>

class QThread;
class QTimer;

namespace Protocol {
    struct ControlPacket;
}

namespace Host::Models {
    struct DeviceManifest;
    struct ConnectionInfo;
}
namespace Host::Processing { class AudioFrameWorker; }
namespace Host::Backend { class IDeviceBackend; }

namespace Host::Core {

/// @brief Central controller managing UI-to-hardware communication hub.
class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(std::unique_ptr<Host::Backend::IDeviceBackend> backend,
                           QObject* parent = nullptr);
    ~AppController() override;

    void setParameter(uint8_t slotId, uint8_t paramId, float value);
    void setEffectType(uint8_t slotId, uint8_t effectTypeId);
    void setBypass(uint8_t slotId, bool bypassed);
    void requestSync();
    void connectToDevice(const QString& target = QString());
    void disconnectFromDevice();

signals:
    void manifestReady(const Host::Models::DeviceManifest& manifest);
    void deviceStateUpdated(const Protocol::ControlPacket& pkt);
    void connectionChanged(bool connected, const QString& portName);
    void connectionsUpdated(const std::vector<Host::Models::ConnectionInfo>& connections);
    void errorOccurred(const QString &errorMessage);
    void spectrumReady(const std::vector<float>& magnitudeDb);

private slots:
    void onConnectionStatusChanged(bool connected, const QString& portName);
    void refreshAvailableConnections();

private:
    void setupBackend();
    void dispatchControlPacket(Protocol::ControlPacket pkt);

    QThread* m_workerThread{nullptr};
    QTimer* m_scanTimer{nullptr};
    Host::Processing::AudioFrameWorker* m_audioWorker{nullptr};
    std::unique_ptr<Host::Backend::IDeviceBackend> m_backend;
    bool m_isDeviceConnected{false};
};

} // namespace Host::Core

#endif // EMBEDDEDDSP_HOST_APPCONTROLLER_H
