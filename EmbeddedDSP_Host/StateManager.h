#ifndef EMBEDDEDDSP_HOST_STATEMANAGER_H
#define EMBEDDEDDSP_HOST_STATEMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QThread>
#include "DeviceInterface.h"
#include "EffectSpec.h"
#include "AudioFrameSimulator.h"
#include "AudioAnalyzer.h"

namespace Host {

/**
 * @brief Central ViewModel for EmbeddedDSP Host.
 * Acts as the source of truth for device state and orchestrates communication.
 */
class StateManager : public QObject {
    Q_OBJECT
public:
    explicit StateManager(QObject* parent = nullptr);
    ~StateManager() override;

    /// @brief Returns the list of effects supported by the device.
    const QMap<int, EffectSpec>& availableEffects() const { return m_availableEffects; }

    /// @brief Returns the number of effect slots on the device.
    int slotCount() const { return m_slotCount; }

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
    /// @brief Emitted when the manifest is ready to rebuild the rack.
    void manifestReady();

    /// @brief Emitted when a control packet is received from the device.
    void deviceStateUpdated(const Protocol::ControlPacket& pkt);

    /// @brief Emitted when the connection status changes.
    void connectionChanged(bool connected, const QString& portName);

    /// @brief Forwarded audio frame from the device.
    void audioFrameReady(const Protocol::AudioFramePacket &frame);

    /// @brief Forwarded error message from the device interface.
    void errorOccurred(const QString &errorMessage);

    /// @brief Emitted when a new frequency spectrum is ready for visualization.
    void spectrumReady(const std::vector<float>& magnitudeDb);

    /// @brief Emitted when the demo status changes.
    void demoRunningChanged(bool running);

private slots:
    void onDeviceManifestReady(const DeviceManifest& manifest);
    void onControlPacketReady(const Protocol::ControlPacket& pkt);
    void onConnectionChanged(bool connected, const QString& portName);

private:
    QThread* m_workerThread{nullptr};
    DeviceInterface* m_deviceInterface{nullptr};
    AudioAnalyzer* m_audioAnalyzer{nullptr};
    AudioFrameSimulator* m_simulator{nullptr};
    QMap<int, EffectSpec> m_availableEffects;
    int m_slotCount{0};
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_STATEMANAGER_H
