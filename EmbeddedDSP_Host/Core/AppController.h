#ifndef EMBEDDEDDSP_HOST_APPCONTROLLER_H
#define EMBEDDEDDSP_HOST_APPCONTROLLER_H

#include <QObject>
#include <QString>
#include <memory>
#include <vector>
#include "DeviceModel.h"
#include "ConnectionInfo.h"

class QThread;
class QTimer;

namespace Protocol {
    struct ControlPacket;
}

namespace Host::Processing { class AudioFrameWorker; }
namespace Host::Backend { class IDeviceBackend; }

namespace Host::Core {

/// @brief Central controller managing UI-to-hardware communication hub.
class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(std::unique_ptr<Host::Backend::IDeviceBackend> backend,
                           std::unique_ptr<Host::Processing::AudioFrameWorker> audioWorker,
                           QObject* parent = nullptr);
    ~AppController() override;

    void setParameter(int slotIndex, int paramIndex, float value);
    void setEffectType(int slotIndex, int effectTypeId);
    void setBypass(int slotIndex, bool bypassed);
    void requestSync();
    void connectToDevice(const QString& target = QString());
    void disconnectFromDevice();

signals:
    void manifestReady(const Host::Models::DeviceManifest& manifest);
    void slotParameterUpdated(int slotIndex, int paramIndex, float value);
    void slotEffectTypeUpdated(int slotIndex, int effectTypeId);
    void slotBypassUpdated(int slotIndex, bool bypassed);
    void connectionChanged(bool connected, const QString& portName);
    void connectionsUpdated(const std::vector<Host::Models::ConnectionInfo>& connections);
    void errorOccurred(const QString &errorMessage);
    void spectrumReady(const std::vector<float>& magnitudeDb);

private slots:
    void onConnectionStatusChanged(bool connected, const QString& portName);
    void onControlPacketReady(const Protocol::ControlPacket& pkt);
    void refreshAvailableConnections();

private:
    void setupBackend();
    void dispatchControlPacket(Protocol::ControlPacket pkt);

    QThread* m_workerThread{nullptr};
    QTimer* m_scanTimer{nullptr};
    
    // Raw pointers for thread-safe access; ownership managed by QThread::finished -> deleteLater
    Host::Processing::AudioFrameWorker* m_audioWorker{nullptr};
    Host::Backend::IDeviceBackend* m_backend{nullptr};
    
    bool m_isDeviceConnected{false};
};

} // namespace Host::Core

#endif // EMBEDDEDDSP_HOST_APPCONTROLLER_H
