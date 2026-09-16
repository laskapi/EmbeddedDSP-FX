#include "EffectSlot.h"
#include "EffectModule.h"
#include <QDebug>
#include <utility>

namespace UI {

EffectSlot::EffectSlot(uint8_t slotId, QWidget *parent)
    : QGroupBox(parent), m_slotId(slotId) {
    setTitle(tr("Slot %1").arg(slotId + 1));
    setupUi();
}

void EffectSlot::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    
    m_typeCombo = new QComboBox(this);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EffectSlot::onTypeChanged);

    m_bypassBtn = new QPushButton(tr("Bypass"), this);
    m_bypassBtn->setCheckable(true);
    connect(m_bypassBtn, &QPushButton::toggled, this, &EffectSlot::onBypassToggled);

    m_mainLayout->addWidget(m_typeCombo);
    // Stretch to keep combo at top and bypass at bottom when no module
    m_mainLayout->addStretch(1); 
    m_mainLayout->addWidget(m_bypassBtn);
}

void EffectSlot::setAvailableEffects(const QMap<int, Host::EffectSpec> &availableSpecs) {
    m_availableSpecs = availableSpecs;
    
    m_typeCombo->blockSignals(true);
    m_typeCombo->clear();
    for (const auto &spec : std::as_const(m_availableSpecs)) {
        m_typeCombo->addItem(spec.name, spec.id);
    }
    m_typeCombo->blockSignals(false);
}

void EffectSlot::onTypeChanged(int index) {
    uint8_t effectId = static_cast<uint8_t>(m_typeCombo->itemData(index).toInt());
    
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::SetEffectType;
    pkt.slotId = m_slotId;
    pkt.effectTypeId = effectId;
    pkt.applyCRC();
    
    emit sendPacketRequested(pkt);
}

void EffectSlot::buildParamsUi(uint8_t effectId) {
    if (m_activeModule) {
        m_activeModule->deleteLater();
        m_activeModule = nullptr;
    }

    if (!m_availableSpecs.contains(effectId)) return;
    const auto &spec = m_availableSpecs[effectId];

    m_activeModule = new EffectModule(spec, this);
    connect(m_activeModule, &EffectModule::paramChanged, this, &EffectSlot::onParamChanged);
    
    // Insert after combo box (index 0)
    m_mainLayout->insertWidget(1, m_activeModule);
}

void EffectSlot::updateFromPacket(const Protocol::ControlPacket &pkt) {
    if (pkt.command == Protocol::Command::SetEffectType) {
        int index = m_typeCombo->findData(pkt.effectTypeId);
        if (index != -1) {
            m_typeCombo->blockSignals(true);
            m_typeCombo->setCurrentIndex(index);
            m_typeCombo->blockSignals(false);
            buildParamsUi(pkt.effectTypeId);
        }
    } else if (pkt.command == Protocol::Command::BypassToggle) {
        m_bypassBtn->blockSignals(true);
        m_bypassBtn->setChecked(pkt.getValue() > 0.5f);
        m_bypassBtn->blockSignals(false);
    } else if (pkt.command == Protocol::Command::SetParam) {
        if (m_activeModule) {
            m_activeModule->updateParam(pkt.paramId, pkt.getValue());
        }
    }
}

void EffectSlot::onParamChanged(uint8_t paramId, float value) {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::SetParam;
    pkt.slotId = m_slotId; 
    pkt.paramId = paramId; 
    pkt.setValue(value);
    pkt.applyCRC();
    
    emit sendPacketRequested(pkt);
}

void EffectSlot::onBypassToggled() {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::BypassToggle;
    pkt.slotId = m_slotId;
    pkt.setValue(m_bypassBtn->isChecked() ? 1.0f : 0.0f);
    pkt.applyCRC();
    emit sendPacketRequested(pkt);
}

} // namespace UI
