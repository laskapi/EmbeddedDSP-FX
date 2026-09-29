#include "EffectRack.h"
#include "EffectWidget.h"
#include "AppController.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

namespace Host::UI {

EffectRack::EffectRack(std::shared_ptr<AppController> controller, QWidget *parent) 
    : SynchronizedRack(parent), m_appController(controller) {
    Q_ASSERT(m_appController);

    m_welcomeLabel = new QLabel(tr("Please connect your EmbeddedDSP device..."), this);
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setStyleSheet("font-size: 16px; color: #777; font-weight: bold;");
    layout()->addWidget(m_welcomeLabel);

    connect(m_appController.get(), &AppController::manifestReady, this, &EffectRack::onManifestReady);
    connect(m_appController.get(), &AppController::deviceStateUpdated, this, &EffectRack::syncFromDevice);
    connect(m_appController.get(), &AppController::connectionChanged, this, [this](bool connected, const QString&) {
        if (!connected) clear();
    });
}

void EffectRack::onManifestReady(const Host::DeviceManifest& manifest) {
    if (!isRackEmpty()) return;

    m_welcomeLabel->hide();

    for (int i = 0; i < manifest.slotCount; ++i) {
        auto *widget = new EffectWidget(this);
        widget->setAvailableEffects(manifest.availableEffects);
        widget->setMinimumHeight(350);
        
        connect(widget, &EffectWidget::parameterChanged, this, [this](int slotIdx, int paramIdx, float val){
            m_appController->setParameter(static_cast<uint8_t>(slotIdx), static_cast<uint8_t>(paramIdx), val);
        });

        connect(widget, &EffectWidget::typeChanged, this, [this](int slotIdx, uint8_t effectId){
            m_appController->setEffectType(static_cast<uint8_t>(slotIdx), effectId);
        });

        connect(widget, &EffectWidget::bypassToggled, this, [this](int slotIdx, bool bypassed){
            m_appController->setBypass(static_cast<uint8_t>(slotIdx), bypassed);
        });

        addSynchronized(widget, 1);
    }
}

void EffectRack::clear() {
    clearSynchronized();
    m_welcomeLabel->show();
}

void EffectRack::syncFromDevice(const Protocol::ControlPacket &pkt) {
    if (auto *widget = getSynchronized(pkt.slotId)) {
        widget->updateFromPacket(pkt);
    }
}

} // namespace Host::UI
