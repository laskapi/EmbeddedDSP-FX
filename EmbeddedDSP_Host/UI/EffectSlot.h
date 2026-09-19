#ifndef EMBEDDEDDSP_HOST_EFFECTSLOT_H
#define EMBEDDEDDSP_HOST_EFFECTSLOT_H

#include <QGroupBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMap>
#include <Protocol/ControlPacket.h>
#include "EffectSpec.h"

namespace UI {

class ParameterPanel;
class StateManager;

/**
 * @brief UI component representing a single effect slot in the rack.
 * 
 * Manages the effect type selection, bypass control, and hosts the
 * dynamic ParameterPanel for the selected effect.
 */
class EffectSlot : public QGroupBox {
    Q_OBJECT
public:
    /**
     * @brief Constructor for EffectSlot.
     * @param slotId Unique index of the slot.
     * @param manager Pointer to the central StateManager.
     * @param parent Pointer to the parent QObject.
     */
    explicit EffectSlot(uint8_t slotId, StateManager* manager, QWidget *parent = nullptr);

    /**
     * @brief Updates the slot state from a control packet.
     * @param pkt Packet received from the device.
     */
    void updateFromPacket(const Protocol::ControlPacket &pkt);

    /** @brief Populates the effect selection combo box with available specs. */
    void setAvailableEffects(const QMap<int, Host::EffectSpec> &availableSpecs);

private slots:
    /** @brief Handles effect type changes from the user. */
    void onTypeChanged(int index);

    /** @brief Handles bypass toggle from the user. */
    void onBypassToggled();

private:
    /** @brief Initializes the static UI elements of the slot. */
    void setupUi();

    /**
     * @brief Configures the slot for a specific effect ID.
     * @param effectId ID of the effect from the catalog.
     */
    void setupEffect(uint8_t effectId);

    uint8_t m_slotId;
    StateManager* m_manager{nullptr};
    QComboBox *m_typeCombo{nullptr};
    QVBoxLayout *m_mainLayout{nullptr};
    ParameterPanel *m_activePanel{nullptr};
    QPushButton *m_bypassBtn{nullptr};

    QMap<int, Host::EffectSpec> m_availableSpecs;
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_EFFECTSLOT_H
