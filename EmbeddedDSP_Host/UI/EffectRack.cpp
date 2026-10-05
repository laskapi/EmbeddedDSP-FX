#include "EffectRack.h"
#include "EffectItem.h"
#include "AppController.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

namespace Host::UI {

EffectRack::EffectRack(std::shared_ptr<AppController> controller, QWidget *parent) 
    : AbstractRack(parent), m_appController(controller) {
    Q_ASSERT(m_appController);

    connect(m_appController.get(), &AppController::manifestReady, this, &EffectRack::onManifestReady);
    connect(m_appController.get(), &AppController::deviceStateUpdated, this, &EffectRack::syncFromDevice);

    connect(m_appController.get(), &AppController::connectionChanged, this, [this](bool connected, const QString&) {
        // clear() will be called by MainWindow after animation ends
    });
}

void EffectRack::onManifestReady(const Host::DeviceManifest& manifest) {
    clearRack();

    for (int i = 0; i < manifest.slotCount; ++i) {
        auto *item = new EffectItem(this);
        item->setAvailableEffects(manifest.availableEffects);
        
        connect(item, &EffectItem::parameterChanged, this, [this](int slotIdx, int paramIdx, float val){
            m_appController->setParameter(static_cast<uint8_t>(slotIdx), static_cast<uint8_t>(paramIdx), val);
        });

        connect(item, &EffectItem::typeChanged, this, [this](int slotIdx, uint8_t effectId){
            m_appController->setEffectType(static_cast<uint8_t>(slotIdx), effectId);
        });

        connect(item, &EffectItem::bypassToggled, this, [this](int slotIdx, bool bypassed){
            m_appController->setBypass(static_cast<uint8_t>(slotIdx), bypassed);
        });

        addItem(item, 1);
    }

    m_appController->requestSync();
}

void EffectRack::clear() {
    clearRack();
}

void EffectRack::syncFromDevice(const Protocol::ControlPacket &pkt) {
    if (auto *item = getItem(pkt.slotId)) {
        item->updateFromPacket(pkt);
    }
}

} // namespace Host::UI
