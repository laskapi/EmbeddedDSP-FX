#ifndef EMBEDDEDDSP_HOST_STATEMANAGER_H
#define EMBEDDEDDSP_HOST_STATEMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include "SerialManager.h"
#include "EffectSpec.h"

namespace UI {

/**
 * @brief Central state manager (ViewModel) for the EmbeddedDSP Host.
 * 
 * Acts as the source of truth for the device state, manages communication
 * via SerialManager, and provides an API for UI components.
 */
class StateManager : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Constructor for StateManager.
     * @param parent Pointer to the parent QObject.
     */
    explicit StateManager(QObject* parent = nullptr);

    /** 
     * @brief Returns the catalog of available effects.
     * @return Constant map of effect IDs to specifications.
     */
    const QMap<int, Host::EffectSpec>& catalog() const { return m_catalog; }

    /** 
     * @brief Returns the number of effect slots on the device.
     * @return Number of slots.
     */
    int slotCount() const { return m_slotCount; }

    /** 
     * @brief Returns reference to the internal SerialManager.
     * @return SerialManager instance.
     */
    SerialManager& serial() { return m_serial; }

    /**
     * @brief Requests a parameter change for a specific slot.
     * @param slotId Index of the effect slot.
     * @param paramId Index of the parameter within the effect.
     * @param value New normalized value.
     */
    void setParameter(uint8_t slotId, uint8_t paramId, float value);

    /**
     * @brief Requests a change of effect type for a specific slot.
     * @param slotId Index of the effect slot.
     * @param effectTypeId ID of the new effect type from the catalog.
     */
    void setEffectType(uint8_t slotId, uint8_t effectTypeId);

    /**
     * @brief Requests a bypass toggle for a specific slot.
     * @param slotId Index of the effect slot.
     * @param bypassed True to bypass, false to enable.
     */
    void setBypass(uint8_t slotId, bool bypassed);

    /** @brief Requests full state synchronization from the device. */
    void requestSync();

signals:
    /** @brief Emitted when the manifest is received and parsed. */
    void manifestProcessed();

    /** @brief Emitted when a control packet is received from the device. */
    void deviceStateUpdated(const Protocol::ControlPacket& pkt);

    /** @brief Emitted when the connection status changes.
     *  @param connected True if device is connected.
     *  @param portName Name of the connected serial port.
     */
    void connectionChanged(bool connected, const QString& portName);

private slots:
    void onManifestReceived(const QString& manifest);
    void onControlPacketReceived(const Protocol::ControlPacket& pkt);

private:
    /**
     * @brief Internal manifest parser.
     * @param manifest Raw manifest string from device.
     */
    void parseManifest(const QString& manifest);

    SerialManager m_serial;
    QMap<int, Host::EffectSpec> m_catalog;
    int m_slotCount{0};
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_STATEMANAGER_H
