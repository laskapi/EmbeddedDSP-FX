#include "EffectSlot.h"
#include "ParameterPanel.h"
#include "StateManager.h"
#include <QDebug>
#include <utility>

namespace UI {

EffectSlot::EffectSlot(uint8_t slotId, StateManager* manager, QWidget *parent)
    : QGroupBox(parent), m_slotId(slotId), m_manager(manager) {
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
    if (m_manager) {
        m_manager->setEffectType(m_slotId, effectId);
    }
}

void EffectSlot::setupEffect(uint8_t effectId) {
    if (m_activePanel) {
        m_activePanel->deleteLater();
        m_activePanel = nullptr;
    }

    if (!m_availableSpecs.contains(effectId)) return;
    const auto &spec = m_availableSpecs[effectId];

    m_activePanel = new ParameterPanel(spec, m_slotId, m_manager, this);
    
    // Insert after combo box (index 0)
    m_mainLayout->insertWidget(1, m_activePanel);
}

void EffectSlot::updateFromPacket(const Protocol::ControlPacket &pkt) {
    if (pkt.command == Protocol::Command::SetEffectType) {
        int index = m_typeCombo->findData(pkt.effectTypeId);
        if (index != -1) {
            m_typeCombo->blockSignals(true);
            m_typeCombo->setCurrentIndex(index);
            m_typeCombo->blockSignals(false);
            setupEffect(pkt.effectTypeId);
        }
    } else if (pkt.command == Protocol::Command::BypassToggle) {
        m_bypassBtn->blockSignals(true);
        m_bypassBtn->setChecked(pkt.getValue() > 0.5f);
        m_bypassBtn->blockSignals(false);
    } else if (pkt.command == Protocol::Command::SetParam) {
        if (m_activePanel) {
            m_activePanel->updateParam(pkt.paramId, pkt.getValue());
        }
    }
}

void EffectSlot::onBypassToggled() {
    if (m_manager) {
        m_manager->setBypass(m_slotId, m_bypassBtn->isChecked());
    }
}

} // namespace UI
