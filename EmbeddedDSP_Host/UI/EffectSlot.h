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

class EffectModule;

class EffectSlot : public QGroupBox {
    Q_OBJECT
public:
    explicit EffectSlot(uint8_t slotId, QWidget *parent = nullptr);
    void updateFromPacket(const Protocol::ControlPacket &pkt);
    void setAvailableEffects(const QMap<int, Host::EffectSpec> &availableSpecs);

signals:
    void sendPacketRequested(const Protocol::ControlPacket &pkt);

private slots:
    void onTypeChanged(int index);
    void onParamChanged(uint8_t paramId, float value);
    void onBypassToggled();

private:
    void setupUi();
    void buildParamsUi(uint8_t effectId);

    uint8_t m_slotId;
    QComboBox *m_typeCombo;
    QVBoxLayout *m_mainLayout;
    EffectModule *m_activeModule = nullptr;
    QPushButton *m_bypassBtn;

    QMap<int, Host::EffectSpec> m_availableSpecs;
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_EFFECTSLOT_H
