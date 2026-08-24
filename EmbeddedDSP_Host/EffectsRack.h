#ifndef EMBEDDEDDSP_HOST_EFFECTSRACK_H
#define EMBEDDEDDSP_HOST_EFFECTSRACK_H

#include <QWidget>
#include <QHBoxLayout>
#include <vector>
#include "EffectWidget.h"
#include "../EmbeddedDSP_Firmware/App/Protocol/EffectParams.h"

class EffectsRack : public QWidget {
    Q_OBJECT
public:
    explicit EffectsRack(QWidget *parent = nullptr);
signals:
    void controlPacketReady(const ControlPacket::ControlPacket &packet);
private:
    std::vector<EffectWidget*> m_slots;
};

#endif // EMBEDDEDDSP_HOST_EFFECTSRACK_H