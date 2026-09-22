#ifndef EMBEDDEDDSP_HOST_EFFECTSLOT_H
#define EMBEDDEDDSP_HOST_EFFECTSLOT_H

#include <QGroupBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMap>
#include <Protocol/ControlPacket.h>
#include "EffectSpec.h"

namespace Host {
    class StateManager;
    namespace UI {

class ParameterPanel;

/**
 * @brief UI component representing a single effect slot in the rack.
 * Manages type selection, bypass control, and hosts the ParameterPanel.
 */
class EffectSlot : public QGroupBox {
    Q_OBJECT
public:
    explicit EffectSlot(uint8_t slotId, StateManager* manager, QWidget *parent = nullptr);

    /// @brief Syncs slot state from an incoming control packet.
    void updateFromPacket(const Protocol::ControlPacket &pkt);

    /// @brief Populates the effect selection combo box.
    void setAvailableEffects(const QMap<int, Host::EffectSpec> &availableEffects);

private slots:
    void onTypeChanged(int index);
    void onBypassToggled();

private:
    void setupUi();
    
    /// @brief Loads a specific effect and builds its parameter panel.
    void setupEffect(uint8_t effectId);

    uint8_t m_slotId;
    StateManager* m_manager{nullptr};
    QComboBox *m_typeCombo{nullptr};
    QVBoxLayout *m_mainLayout{nullptr};
    ParameterPanel *m_activePanel{nullptr};
    QPushButton *m_bypassBtn{nullptr};

    QMap<int, Host::EffectSpec> m_availableEffects;
};

} // namespace UI
} // namespace Host

#endif // EMBEDDEDDSP_HOST_EFFECTSLOT_H
