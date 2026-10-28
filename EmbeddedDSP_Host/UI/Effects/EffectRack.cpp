#include "UI/Effects/EffectRack.h"
#include "UI/Effects/EffectItem.h"
#include "Core/AppController.h"
#include <QHBoxLayout>

using namespace Host::Models;
using namespace Host::Core;

namespace Host::UI::Effects {

EffectRack::EffectRack(AppController& controller, QWidget *parent)
    : AbstractRack(parent), m_appController(controller) {

    connect(&m_appController, &AppController::manifestReady, this, &EffectRack::onManifestReady);
    connect(&m_appController, &AppController::slotParameterUpdated, this, &EffectRack::onSlotParameterUpdated);
    connect(&m_appController, &AppController::slotEffectTypeUpdated, this, &EffectRack::onSlotEffectTypeUpdated);
    connect(&m_appController, &AppController::slotBypassUpdated, this, &EffectRack::onSlotBypassUpdated);
}

void EffectRack::onManifestReady(const DeviceManifest& manifest) {
    clearRack();
    m_syncedSlotsCount = 0;
    m_expectedSlotsCount = manifest.slotCount;
    m_isSyncing = true;

    for (int i = 0; i < manifest.slotCount; ++i) {
        auto *item = createItem(this);
        item->setAvailableEffects(manifest.availableEffects);

        connect(item, &EffectItem::parameterChanged, &m_appController, &AppController::setParameter);
        connect(item, &EffectItem::typeChanged, &m_appController, &AppController::setEffectType);
        connect(item, &EffectItem::bypassToggled, &m_appController, &AppController::setBypass);
    }

    m_appController.requestSync();
}

void EffectRack::clear() {
    clearRack();
}

void EffectRack::onSlotParameterUpdated(int slotIndex, int paramIndex, float value) {
    if (auto *item = getItem(slotIndex)) {
        item->applyParameter(paramIndex, value);
    }
}

void EffectRack::onSlotEffectTypeUpdated(int slotIndex, int effectTypeId) {
    if (auto *item = getItem(slotIndex)) {
        item->applyEffectType(effectTypeId);

        if (m_isSyncing) {
            m_syncedSlotsCount++;
            if (m_syncedSlotsCount >= m_expectedSlotsCount) {
                m_isSyncing = false;
                emit rackReady();
            }
        }
    }
}

void EffectRack::onSlotBypassUpdated(int slotIndex, bool bypassed) {
    if (auto *item = getItem(slotIndex)) {
        item->applyBypass(bypassed);
    }
}

} // namespace Host::UI::Effects
