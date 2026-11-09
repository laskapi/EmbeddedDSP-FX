#include "UI/Effects/EffectRack.h"
#include "UI/Effects/EffectChainArrow.h"
#include "UI/Effects/EffectItem.h"
#include "Core/AppController.h"
#include <QHBoxLayout>
#include <algorithm>

using namespace Host::Models;
using namespace Host::Core;

namespace Host::UI::Effects {

EffectRack::EffectRack(AppController& controller, QWidget *parent)
    : AbstractRack(parent), m_appController(controller) {
    layout()->setSpacing(0);

    connect(&m_appController, &AppController::manifestReady, this, &EffectRack::onManifestReady);
    connect(&m_appController, &AppController::slotParameterUpdated, this, &EffectRack::onSlotParameterUpdated);
    connect(&m_appController, &AppController::slotEffectTypeUpdated, this, &EffectRack::onSlotEffectTypeUpdated);
    connect(&m_appController, &AppController::slotBypassUpdated, this, &EffectRack::onSlotBypassUpdated);
}

void EffectRack::onManifestReady(const DeviceManifest& manifest) {
    clear();
    m_syncedSlotsCount = 0;
    m_expectedSlotsCount = manifest.slotCount;
    m_isSyncing = true;

    int maxParamCount = 0;
    for (const auto& spec : manifest.availableEffects) {
        maxParamCount = std::max(maxParamCount, static_cast<int>(spec.params.size()));
    }

    for (int i = 0; i < manifest.slotCount; ++i) {
        if (i > 0) {
            layout()->addWidget(new EffectChainArrow(this), 0);
        }

        auto *item = createItem(this);
        item->setAvailableEffects(manifest.availableEffects);
        item->lockHeightForParamCapacity(maxParamCount);
        layout()->setStretchFactor(item, 1);

        connect(item, &EffectItem::parameterChanged, &m_appController, &AppController::setParameter);
        connect(item, &EffectItem::typeChanged, &m_appController, &AppController::setEffectType);
        connect(item, &EffectItem::bypassToggled, &m_appController, &AppController::setBypass);
    }
}

void EffectRack::clear() {
    clearRack();
    while (QLayoutItem* child = layout()->takeAt(0)) {
        delete child->widget();
        delete child;
    }
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
