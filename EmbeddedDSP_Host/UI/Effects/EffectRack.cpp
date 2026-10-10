#include "EffectRack.h"
#include "EffectItem.h"
#include "AppController.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

using namespace Host::Models;

namespace Host::UI::Effects {

EffectRack::EffectRack(Host::Core::AppController& controller, QWidget *parent)
    : AbstractRack(parent), m_appController(controller) {

    connect(&m_appController, &Host::Core::AppController::manifestReady, this, &EffectRack::onManifestReady);
    connect(&m_appController, &Host::Core::AppController::deviceStateUpdated, this, &EffectRack::syncFromDevice);

    connect(&m_appController, &Host::Core::AppController::connectionChanged, this, [this](bool connected, const QString&) {
        // clear() will be called by MainWindow after animation ends
        (void)connected;
    });
}

void EffectRack::onManifestReady(const DeviceManifest& manifest) {
    clearRack();
    m_syncedSlotsCount = 0;
    m_expectedSlotsCount = manifest.slotCount;
    m_isSyncing = true;

    for (int i = 0; i < manifest.slotCount; ++i) {
        auto *item = createItem(this);
        item->setAvailableEffects(manifest.availableEffects);

        connect(item, &EffectItem::parameterChanged, this, [this](int slotIdx, int paramIdx, float val) {
            m_appController.setParameter(static_cast<uint8_t>(slotIdx), static_cast<uint8_t>(paramIdx), val);
        });

        connect(item, &EffectItem::typeChanged, this, [this](int slotIdx, uint8_t effectId) {
            m_appController.setEffectType(static_cast<uint8_t>(slotIdx), effectId);
        });

        connect(item, &EffectItem::bypassToggled, this, [this](int slotIdx, bool bypassed) {
            m_appController.setBypass(static_cast<uint8_t>(slotIdx), bypassed);
        });
    }

    m_appController.requestSync();
}

void EffectRack::clear() {
    clearRack();
}

void EffectRack::syncFromDevice(const Protocol::ControlPacket &pkt) {
    if (auto *item = getItem(pkt.slotId)) {
        bool wasTypePacket = (pkt.command == Protocol::Command::SetEffectType);
        item->updateFromPacket(pkt);

        if (m_isSyncing && wasTypePacket) {
            m_syncedSlotsCount++;
            if (m_syncedSlotsCount >= m_expectedSlotsCount) {
                m_isSyncing = false;
                emit rackReady();
            }
        }
    }
}

} // namespace Host::UI::Effects
