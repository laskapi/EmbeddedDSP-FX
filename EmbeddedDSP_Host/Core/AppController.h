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
namespace Host::Device {
class IDevice;
class IDeviceFactory;
class IConnectionScanner;
class DeviceSession;
}

namespace Host::Core {

/// @brief Central controller managing UI-to-device communication hub.
class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(std::unique_ptr<Host::Device::IConnectionScanner> scanner,
                           std::unique_ptr<Host::Device::IDeviceFactory> factory,
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
    void errorOccurred(const QString& errorMessage);
    void spectrumReady(const std::vector<float>& magnitudeDb);

private slots:
    void onConnectionStatusChanged(bool connected, const QString& portName);
    void onControlPacketReady(const Protocol::ControlPacket& pkt);
    void onDeviceReady(Host::Device::IDevice* device);
    void refreshAvailableConnections();

private:
    void setupDeviceSignals(Host::Device::IDevice* device);
    void dispatchControlPacket(Protocol::ControlPacket pkt);

    QThread* m_workerThread{nullptr};
    QTimer* m_scanTimer{nullptr};

    std::unique_ptr<Host::Device::IConnectionScanner> m_scanner;
    std::unique_ptr<Host::Device::IDeviceFactory> m_factory;
    std::unique_ptr<Host::Device::DeviceSession> m_session;

    Host::Processing::AudioFrameWorker* m_audioWorker{nullptr};
    Host::Device::IDevice* m_device{nullptr};

    bool m_isDeviceConnected{false};
};

} // namespace Host::Core

#endif // EMBEDDEDDSP_HOST_APPCONTROLLER_H
