#ifndef EMBEDDEDDSP_HOST_EFFECTITEM_H
#define EMBEDDEDDSP_HOST_EFFECTITEM_H

#include "AbstractRackItem.h"
#include "EffectSpec.h"
#include <Protocol/ControlPacket.h>
#include <QMap>

class QComboBox;
class QPushButton;
class QVBoxLayout;
class QLabel;

namespace Host::UI {

class ParameterRack;

/// @brief UI component for a single effect in the chain.
class EffectItem : public AbstractRackItem {
    Q_OBJECT
public:
    explicit EffectItem(int index, QWidget *parent = nullptr);

    /// @brief Syncs widget state from device data.
    void updateFromPacket(const Protocol::ControlPacket &pkt);

    /// @brief Sets the list of available effects for the combo box.
    void setAvailableEffects(const QMap<int, Host::EffectSpec> &availableEffects);

signals:
    void parameterChanged(int slotIdx, int paramIdx, float newValue);
    void typeChanged(int slotIdx, uint8_t effectId);
    void bypassToggled(int slotIdx, bool bypassed);

private slots:
    void onTypeChanged(int index);
    void onBypassToggled();

private:
    void setupUi();
    void setupEffect(uint8_t effectId);

    QLabel *m_titleLabel{nullptr};
    QComboBox *m_typeCombo{nullptr};
    ParameterRack *m_parameterRack{nullptr};
    QPushButton *m_bypassBtn{nullptr};
    QVBoxLayout *m_mainLayout{nullptr};
    
    QMap<int, Host::EffectSpec> m_availableEffects;
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_EFFECTITEM_H
