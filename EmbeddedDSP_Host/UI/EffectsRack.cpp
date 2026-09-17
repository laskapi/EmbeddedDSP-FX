#include "EffectsRack.h"
#include "EffectSlot.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

namespace UI {

EffectsRack::EffectsRack(QWidget *parent) : QWidget(parent) {
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_welcomeLabel = new QLabel(tr("Please connect your EmbeddedDSP device..."), this);
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setStyleSheet("font-size: 16px; color: #777; font-weight: bold;");
    mainLayout->addWidget(m_welcomeLabel);

    m_rack = new SlotRack(this);
    mainLayout->addWidget(m_rack);
    m_rack->hide();
}

void EffectsRack::onManifestReceived(const QString &manifest) {
    if (!m_rack->isEmpty()) return;
    
    m_welcomeLabel->hide();
    m_rack->show();

    QMap<int, Host::EffectSpec> availableSpecs;
    int slotCount = 0;
    const QStringList effectLines = manifest.split('\n', Qt::SkipEmptyParts);

    for (const QString &effectLine : effectLines) {
        QStringList segments = effectLine.split(';', Qt::SkipEmptyParts);
        if (segments.isEmpty()) continue;

        QStringList header = segments[0].split(':');
        if (header.isEmpty()) continue;

        const QString tag = header[0];

        if (tag == QLatin1String("C")) {
            if (header.size() >= 3 && header[1] == QLatin1String("SLOTS")) {
                slotCount = header[2].toInt();
            }
        } else if (tag == QLatin1String("E")) {
            if (header.size() < 3) continue;

            Host::EffectSpec spec;
            spec.id = static_cast<uint8_t>(header[1].toInt());
            spec.name = header[2];

            auto params = segments.sliced(1);
            for (const QString &param : params) {
                QStringList p = param.split(':');
                if (p.size() < 5 || p[0] != QLatin1String("P")) continue;
                
                spec.params.append({p[1], p[2].toFloat(), p[3].toFloat(), p[4].toFloat()});
            }
            availableSpecs[spec.id] = spec;
        }
    }

    if (slotCount <= 0) slotCount = 4;

    for (uint8_t i = 0; i < slotCount; ++i) {
        auto *slot = new EffectSlot(i, m_rack);
        slot->setAvailableEffects(availableSpecs);
        slot->setMinimumHeight(350);
        m_rack->add(i, slot, 1);
        
        connect(slot, &EffectSlot::sendPacketRequested, this, &EffectsRack::sendPacketRequested);
    }
}

void EffectsRack::clear() {
    m_rack->clear();
    m_rack->hide();
    m_welcomeLabel->show();
}

void EffectsRack::syncFromDevice(const Protocol::ControlPacket &pkt) {
    if (auto *slot = m_rack->get(pkt.slotId)) {
        slot->updateFromPacket(pkt);
    }
}

} // namespace UI
