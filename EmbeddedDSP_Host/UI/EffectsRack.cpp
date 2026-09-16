#include "EffectsRack.h"
#include "EffectSlot.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

namespace UI {

EffectsRack::EffectsRack(QWidget *parent) : QWidget(parent) {
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_welcomeLabel = new QLabel(tr("Please connect your EmbeddedDSP device..."), this);
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setStyleSheet("font-size: 16px; color: #777; font-weight: bold;");
    m_mainLayout->addWidget(m_welcomeLabel);
}

void EffectsRack::onManifestReceived(const QString &manifest) {
    if (!m_slots.empty()) return;
    m_welcomeLabel->hide();

    QMap<int, Host::EffectSpec> availableSpecs;
    int slotCount = 0;
    const QStringList lines = manifest.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        if (line.startsWith(QLatin1String("CONF:"))) {
            if (line.contains(QLatin1String("SLOTS="))) {
                slotCount = line.section('=', 1).toInt();
            }
            continue;
        }

        QStringList parts = line.split(';', Qt::SkipEmptyParts);
        if (parts.isEmpty()) continue;

        QStringList header = parts[0].split(':');
        if (header.size() < 3) continue;

        Host::EffectSpec spec;
        spec.id = static_cast<uint8_t>(header[1].toInt());
        spec.name = header[2];

        for (int i = 1; i < parts.size(); ++i) {
            QStringList p = parts[i].split(':');
            if (p.size() < 5) continue;
            spec.params.append({p[1], p[2].toFloat(), p[3].toFloat(), p[4].toFloat()});
        }
        availableSpecs[spec.id] = spec;
    }

    if (slotCount <= 0) slotCount = 4;

    for (uint8_t i = 0; i < slotCount; ++i) {
        auto *slot = new EffectSlot(i, this);
        slot->setAvailableEffects(availableSpecs);
        slot->setMinimumHeight(350);
        m_slots.push_back(slot);
        m_mainLayout->addWidget(slot, 1);
        
        connect(slot, &EffectSlot::sendPacketRequested, this, &EffectsRack::sendPacketRequested);
    }
}

void EffectsRack::clear() {
    for (auto* slot: m_slots) {
        m_mainLayout->removeWidget(slot);
        delete slot;
    }
    m_slots.clear();
    m_welcomeLabel->show();
}

void EffectsRack::syncFromDevice(const Protocol::ControlPacket &pkt) {
    if (pkt.slotId < static_cast<uint8_t>(m_slots.size())) {
        m_slots[pkt.slotId]->updateFromPacket(pkt);
    }
}

} // namespace UI
