#include "EffectsRack.h"
#include "EffectSlot.h"
#include "StateManager.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

namespace Host::UI {

EffectsRack::EffectsRack(StateManager* manager, QWidget *parent) 
    : QWidget(parent), m_manager(manager) {
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_welcomeLabel = new QLabel(tr("Please connect your EmbeddedDSP device..."), this);
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setStyleSheet("font-size: 16px; color: #777; font-weight: bold;");
    mainLayout->addWidget(m_welcomeLabel);

    m_rack = new SlotRack(this);
    mainLayout->addWidget(m_rack);
    m_rack->hide();

    if (m_manager) {
        connect(m_manager, &StateManager::manifestReady, this, &EffectsRack::onManifestReady);
        connect(m_manager, &StateManager::deviceStateUpdated, this, &EffectsRack::syncFromDevice);
        connect(m_manager, &StateManager::connectionChanged, this, [this](bool connected, const QString&) {
            if (!connected) clear();
        });
    }
}

void EffectsRack::onManifestReady() {
    if (!m_rack->isEmpty()) return;
    if (!m_manager) return;

    m_welcomeLabel->hide();
    m_rack->show();

    int slotCount = m_manager->slotCount();
    const auto& availableEffects = m_manager->availableEffects();

    for (uint8_t i = 0; i < slotCount; ++i) {
        auto *slot = new EffectSlot(i, m_manager, m_rack);
        slot->setAvailableEffects(availableEffects);
        slot->setMinimumHeight(350);
        m_rack->add(i, slot, 1);
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

} // namespace Host::UI
