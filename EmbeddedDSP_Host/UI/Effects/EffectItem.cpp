#include "EffectItem.h"
#include "ParameterRack.h"
#include "UiObjectNames.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

namespace Host::UI::Effects {

EffectItem::EffectItem(int index, QWidget *parent)
    : AbstractRackItem(index, parent) {
    setObjectName(Host::UI::UiObjectNames::kEffectItemBox);
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
    m_titleLabel->setObjectName(Host::UI::UiObjectNames::kEffectTitle);

    m_typeCombo = new QComboBox(this);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EffectItem::onTypeChanged);

    m_bypassBtn = new QPushButton(tr("Bypass"), this);
    m_bypassBtn->setCheckable(true);
    connect(m_bypassBtn, &QPushButton::toggled, this, &EffectItem::onBypassToggled);

    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_typeCombo);
    m_mainLayout->addWidget(m_bypassBtn);
}

void EffectItem::setAvailableEffects(const QMap<int, Host::Models::EffectSpec> &availableEffects) {
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
    emit typeChanged(m_index, m_typeCombo->itemData(index).toInt());
}

void EffectItem::onBypassToggled() {
    emit bypassToggled(m_index, m_bypassBtn->isChecked());
}

void EffectItem::setupEffect(int effectTypeId) {
    if (m_parameterRack) {
        m_parameterRack->deleteLater();
        m_parameterRack = nullptr;
    }

    if (!m_availableEffects.contains(effectTypeId)) return;
    const auto &spec = m_availableEffects[effectTypeId];

    m_parameterRack = new ParameterRack(spec, this);

    connect(m_parameterRack, &ParameterRack::parameterChanged, this, [this](int paramIdx, float val) {
        emit parameterChanged(m_index, paramIdx, val);
    });

    m_mainLayout->insertWidget(2, m_parameterRack);
}

void EffectItem::applyEffectType(int effectTypeId) {
    int index = m_typeCombo->findData(effectTypeId);
    if (index != -1) {
        m_typeCombo->blockSignals(true);
        m_typeCombo->setCurrentIndex(index);
        m_typeCombo->blockSignals(false);
        setupEffect(effectTypeId);
    }
}

void EffectItem::applyBypass(bool bypassed) {
    m_bypassBtn->blockSignals(true);
    m_bypassBtn->setChecked(bypassed);
    m_bypassBtn->blockSignals(false);
}

void EffectItem::applyParameter(int paramIndex, float value) {
    if (m_parameterRack) {
        m_parameterRack->updateParam(paramIndex, value);
    }
}

} // namespace Host::UI::Effects
