#ifndef EMBEDDEDDSP_HOST_EFFECTSRACK_H
#define EMBEDDEDDSP_HOST_EFFECTSRACK_H

#include <QWidget>
#include <vector>
#include <QMap>
#include <Protocol/ControlPacket.h>
#include "../EffectSpec.h"

class QHBoxLayout;
class QLabel;

namespace UI {

class EffectSlot;

class EffectsRack : public QWidget {
    Q_OBJECT
public:
    explicit EffectsRack(QWidget *parent = nullptr);
    void syncFromDevice(const Protocol::ControlPacket &pkt);

public slots:
    void onManifestReceived(const QString &manifest);
    void clear();

signals:
    void sendPacketRequested(const Protocol::ControlPacket &pkt);

private:
    std::vector<EffectSlot*> m_slots;
    QHBoxLayout *m_mainLayout = nullptr;
    QLabel *m_welcomeLabel = nullptr;
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_EFFECTSRACK_H
