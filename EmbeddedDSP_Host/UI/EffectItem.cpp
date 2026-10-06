#include "EffectItem.h"
#include "ParameterRack.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

namespace Host::UI {

EffectItem::EffectItem(int index, QWidget *parent)
    : AbstractRackItem(index, parent) {
    setObjectName("effectItemBox");
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
    
    if (m_titleLabel) {
        m_titleLabel->setText(tr("SLOT %1").arg(m_index + 1));
    }
}

void EffectItem::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(8);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("effectTitle");
    
    m_typeCombo = new QComboBox(this);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EffectItem::onTypeChanged);

    m_bypassBtn = new QPushButton(tr("Bypass"), this);
    m_bypassBtn->setCheckable(true);
    connect(m_bypassBtn, &QPushButton::toggled, this, &EffectItem::onBypassToggled);

    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_typeCombo);
    m_mainLayout->addWidget(m_bypassBtn);
}

void EffectItem::setAvailableEffects(const QMap<int, Host::EffectSpec> &availableEffects) {
    m_availableEffects = availableEffects;
    
    m_typeCombo->blockSignals(true);
    m_typeCombo->clear();
    for (const auto &spec : m_availableEffects) {
        m_typeCombo->addItem(spec.name, spec.id);
    }
    m_typeCombo->blockSignals(false);
}

void EffectItem::onTypeChanged(int index) {
    if (index < 0) return;
    uint8_t effectId = static_cast<uint8_t>(m_typeCombo->itemData(index).toInt());
    emit typeChanged(m_index, effectId);
}

void EffectItem::onBypassToggled() {
    emit bypassToggled(m_index, m_bypassBtn->isChecked());
}

void EffectItem::setupEffect(uint8_t effectId) {
    if (m_parameterRack) {
        m_parameterRack->deleteLater();
        m_parameterRack = nullptr;
    }

    if (!m_availableEffects.contains(effectId)) return;
    const auto &spec = m_availableEffects[effectId];

    m_parameterRack = new ParameterRack(spec, this);
    
    connect(m_parameterRack, &ParameterRack::parameterChanged, this, [this](int paramIdx, float val){
        emit parameterChanged(m_index, paramIdx, val);
    });
    
    m_mainLayout->insertWidget(2, m_parameterRack);
}

void EffectItem::updateFromPacket(const Protocol::ControlPacket &pkt) {
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
        if (m_parameterRack) {
            m_parameterRack->updateParam(pkt.paramId, pkt.getValue());
        }
    }
}

} // namespace Host::UI
