#ifndef EMBEDDEDDSP_HOST_EFFECTSRACK_H
#define EMBEDDEDDSP_HOST_EFFECTSRACK_H

#include <QWidget>
#include <vector>
#include <QMap>
#include <Protocol/ControlPacket.h>
#include "EffectWidget.h"

class QHBoxLayout;
class QLabel;

class EffectsRack : public QWidget {
    Q_OBJECT
public:
    explicit EffectsRack(QWidget *parent = nullptr);
    void syncFromDevice(const Protocol::ControlPacket &pkt);

public slots:
    void onManifestReceived(const QString &manifest);
    void clear();
signals:
    void controlPacketReady(const Protocol::ControlPacket &packet);

private:
    std::vector<EffectWidget*> m_slots;
    QHBoxLayout *m_mainLayout = nullptr;
    QLabel *m_welcomeLabel = nullptr;
};

#endif // EMBEDDEDDSP_HOST_EFFECTSRACK_H
