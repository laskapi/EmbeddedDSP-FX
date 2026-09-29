#ifndef EMBEDDEDDSP_HOST_EFFECTWIDGET_H
#define EMBEDDEDDSP_HOST_EFFECTWIDGET_H

#include "SynchronizedWidget.h"
#include "EffectSpec.h"
#include <Protocol/ControlPacket.h>
#include <QMap>

class QComboBox;
class QPushButton;
class QVBoxLayout;
class QGroupBox;

namespace Host::UI {

class ParameterRack;

/**
 * @brief UI component for a single effect in the chain.
 * Manages its own type selection and bypass. Bubbles signals to EffectRack.
 */
class EffectWidget : public SynchronizedWidget {
    Q_OBJECT
public:
    explicit EffectWidget(QWidget *parent = nullptr);

    /// @brief Override to update group box title when index changes.
    void setLogicalIndex(int index);

    /// @brief Syncs widget state from device data.
    void updateFromPacket(const Protocol::ControlPacket &pkt);

    /// @brief Sets the list of available effects for the combo box.
    void setAvailableEffects(const QMap<int, Host::EffectSpec> &availableEffects);

signals:
    /// @brief Bubbled up when a parameter in this effect changes.
    void parameterChanged(int slotIdx, int paramIdx, float newValue);
    
    /// @brief Bubbled up when the user changes the effect type.
    void typeChanged(int slotIdx, uint8_t effectId);
    
    /// @brief Bubbled up when bypass is toggled.
    void bypassToggled(int slotIdx, bool bypassed);

private slots:
    void onTypeChanged(int index);
    void onBypassToggled();

private:
    void setupUi();
    void setupEffect(uint8_t effectId);

    QGroupBox *m_groupBox{nullptr};
    QComboBox *m_typeCombo{nullptr};
    ParameterRack *m_activeRack{nullptr};
    QPushButton *m_bypassBtn{nullptr};
    QVBoxLayout *m_innerLayout{nullptr};
    
    QMap<int, Host::EffectSpec> m_availableEffects;
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_EFFECTWIDGET_H
