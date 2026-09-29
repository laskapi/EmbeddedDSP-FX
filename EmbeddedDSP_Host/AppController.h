#ifndef EMBEDDEDDSP_HOST_APPCONTROLLER_H
#define EMBEDDEDDSP_HOST_APPCONTROLLER_H

#include <QObject>
#include <QMap>
#include <vector>
#include "EffectSpec.h"

class QThread;

namespace Protocol {
    struct ControlPacket;
    struct AudioFramePacket;
}

namespace Host {

class DeviceInterface;
class AudioFrameWorker;
class AudioFrameSimulator;

/**
 * @brief Central AppController for EmbeddedDSP Host.
 * Acts as a communication hub between the UI and the worker thread (Serial/DSP).
 */
class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController();
    ~AppController() override;

    /// @brief Requests a parameter change for a specific slot.
    void setParameter(uint8_t slotId, uint8_t paramId, float value);

    /// @brief Requests a change of effect type for a specific slot.
    void setEffectType(uint8_t slotId, uint8_t effectTypeId);

    /// @brief Requests a bypass toggle for a specific slot.
    void setBypass(uint8_t slotId, bool bypassed);

    /// @brief Requests full state synchronization from the device.
    void requestSync();

    /// @brief Requests connecting to the device in the worker thread.
    void connect(const QString& target = QString());

    /// @brief Requests disconnecting from the device.
    void disconnect();

    /// @brief Starts the demo audio simulator.
    void startDemo();

    /// @brief Stops the demo audio simulator.
    void stopDemo();

signals:
    /// @brief Emitted when the device manifest is received (describes slots and effects).
    void manifestReady(const Host::DeviceManifest& manifest);

    /// @brief Emitted when a control packet is received from the device.
    void deviceStateUpdated(const Protocol::ControlPacket& pkt);

    /// @brief Emitted when the connection status changes.
    void connectionChanged(bool connected, const QString& portName);

    /// @brief Forwarded audio frame from the device/simulator.
    void audioFrameReady(const Protocol::AudioFramePacket &frame);

    /// @brief Forwarded error message from the device interface.
    void errorOccurred(const QString &errorMessage);

    /// @brief Emitted when a new frequency spectrum is ready for visualization.
    void spectrumReady(const std::vector<float>& magnitudeDb);

    /// @brief Emitted when the demo status changes.
    void demoRunningChanged(bool running);

private slots:
    void onConnectionChanged(bool connected, const QString& portName);

private:
    QThread* m_workerThread{nullptr};
    DeviceInterface* m_deviceInterface{nullptr};
    AudioFrameWorker* m_audioWorker{nullptr};
    AudioFrameSimulator* m_simulator{nullptr};
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_APPCONTROLLER_H
