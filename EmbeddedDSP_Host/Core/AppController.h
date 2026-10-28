#ifndef EMBEDDEDDSP_HOST_APPCONTROLLER_H
#define EMBEDDEDDSP_HOST_APPCONTROLLER_H

#include <QObject>
#include <QString>
#include <memory>
#include <vector>
#include "Models/ConnectionInfo.h"
#include "Models/DeviceModel.h"
#include "Core/WorkerThread.h"

namespace Protocol {
struct ControlPacket;
}

namespace Host::Processing { class AudioFrameWorker; }
namespace Host::Device {
class IConnectionScanner;
class DeviceSession;
}

namespace Host::Core {

/// @brief Central controller managing UI-to-device communication hub.
class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(std::unique_ptr<Host::Device::IConnectionScanner> connectionScanner,
                           std::unique_ptr<Host::Device::DeviceSession> deviceSession,
                           std::unique_ptr<Host::Processing::AudioFrameWorker> audioWorker,
                           QObject* parent = nullptr);
    ~AppController() override;

    void setParameter(int slotIndex, int paramIndex, float value);
    void setEffectType(int slotIndex, int effectTypeId);
    void setBypass(int slotIndex, bool bypassed);
    void requestSync();
    void connectDevice(const QString& connectionId);
    void disconnectDevice();

signals:
    void manifestReady(const Host::Models::DeviceManifest& manifest);
    void slotParameterUpdated(int slotIndex, int paramIndex, float value);
    void slotEffectTypeUpdated(int slotIndex, int effectTypeId);
    void slotBypassUpdated(int slotIndex, bool bypassed);
    void connectionChanged(bool connected, const QString& portName);
    void connectionsUpdated(const std::vector<Host::Models::ConnectionInfo>& connections);
    void errorOccurred(const QString& errorMessage);
    void spectrumReady(const std::vector<float>& magnitudeDb);

private slots:
    void onConnectionStatusChanged(bool connected, const QString& portName);
    void onControlPacketReady(const Protocol::ControlPacket& pkt);

private:
    void dispatchControlPacket(Protocol::ControlPacket pkt);

    /// @brief Declared first so it is destroyed last (quit/wait after adopted objects).
    WorkerThread m_workerThread;

    Host::Device::DeviceSession* m_deviceSession{nullptr};
    Host::Device::IConnectionScanner* m_connectionScanner{nullptr};
    Host::Processing::AudioFrameWorker* m_audioWorker{nullptr};

    bool m_isDeviceConnected{false};
};

} // namespace Host::Core

#endif // EMBEDDEDDSP_HOST_APPCONTROLLER_H
