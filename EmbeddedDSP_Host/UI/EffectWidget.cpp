#include "EffectWidget.h"
#include "ParameterRack.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>

namespace Host::UI {

EffectWidget::EffectWidget(QWidget *parent)
    : SynchronizedWidget(parent) {
    setupUi();
}

void EffectWidget::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_groupBox = new QGroupBox(this);
    mainLayout->addWidget(m_groupBox);

    m_innerLayout = new QVBoxLayout(m_groupBox);
    
    m_typeCombo = new QComboBox(m_groupBox);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EffectWidget::onTypeChanged);

    m_bypassBtn = new QPushButton(tr("Bypass"), m_groupBox);
    m_bypassBtn->setCheckable(true);
    connect(m_bypassBtn, &QPushButton::toggled, this, &EffectWidget::onBypassToggled);

    m_innerLayout->addWidget(m_typeCombo);
    m_innerLayout->addStretch(1); 
    m_innerLayout->addWidget(m_bypassBtn);
}

void EffectWidget::setLogicalIndex(int index) {
    SynchronizedWidget::setLogicalIndex(index);
    if (m_groupBox) {
        m_groupBox->setTitle(tr("Slot %1").arg(index + 1));
    }
}

void EffectWidget::setAvailableEffects(const QMap<int, EffectSpec> &availableEffects) {
    m_availableEffects = availableEffects;
    
    m_typeCombo->blockSignals(true);
    m_typeCombo->clear();
    for (const auto &spec : m_availableEffects) {
        m_typeCombo->addItem(spec.name, spec.id);
    }
    m_typeCombo->blockSignals(false);
}

void EffectWidget::onTypeChanged(int index) {
    uint8_t effectId = static_cast<uint8_t>(m_typeCombo->itemData(index).toInt());
    emit typeChanged(m_logicalIndex, effectId);
}

void EffectWidget::onBypassToggled() {
    emit bypassToggled(m_logicalIndex, m_bypassBtn->isChecked());
}

void EffectWidget::setupEffect(uint8_t effectId) {
    if (m_activeRack) {
        m_activeRack->deleteLater();
        m_activeRack = nullptr;
    }

    if (!m_availableEffects.contains(effectId)) return;
    const auto &spec = m_availableEffects[effectId];

    m_activeRack = new ParameterRack(spec, m_groupBox);
    
    connect(m_activeRack, &ParameterRack::parameterChanged, this, [this](int paramIdx, float val){
        emit parameterChanged(m_logicalIndex, paramIdx, val);
    });
    
    m_innerLayout->insertWidget(1, m_activeRack);
}

void EffectWidget::updateFromPacket(const Protocol::ControlPacket &pkt) {
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
        if (m_activeRack) {
            m_activeRack->updateParam(pkt.paramId, pkt.getValue());
        }
    }
}

} // namespace Host::UI
