#ifndef EMBEDDEDDSP_HOST_EFFECTSRACK_H
#define EMBEDDEDDSP_HOST_EFFECTSRACK_H

#include <QWidget>
#include <QHBoxLayout>
#include <Protocol/ControlPacket.h>
#include "WidgetRack.h"
#include "../EffectSpec.h"

class QLabel;

namespace UI {

class EffectSlot;

class EffectsRack : public QWidget {
    Q_OBJECT
    using SlotRack = WidgetRack<uint8_t, EffectSlot, QHBoxLayout>;
public:
    explicit EffectsRack(QWidget *parent = nullptr);
    void syncFromDevice(const Protocol::ControlPacket &pkt);

public slots:
    void onManifestReceived(const QString &manifest);
    void clear();

signals:
    void sendPacketRequested(const Protocol::ControlPacket &pkt);

private:
    SlotRack* m_rack;
    QLabel *m_welcomeLabel = nullptr;
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_EFFECTSRACK_H
