#include "UI/Effects/EffectItem.h"
#include "UI/Effects/EffectDragHandle.h"
#include "UI/Effects/EqChrome.h"
#include "UI/Effects/ParameterRack.h"
#include "UI/UiObjectNames.h"
#include "Protocol/ProtocolCommon.h"

#include <QComboBox>
#include <QPushButton>
#include <QSizePolicy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace Host::UI::Effects {

namespace {
constexpr int kEmptyEffectId = 0;
}

EffectItem::EffectItem(int index, QWidget *parent)
    : AbstractRackItem(index, parent) {
    setObjectName(Host::UI::UiObjectNames::kEffectItemBox);
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
}

void EffectItem::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 6, 10, 10);
    m_mainLayout->setSpacing(8);

    m_dragHandle = new EffectDragHandle(this, this);

    m_typeCombo = new QComboBox(this);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EffectItem::onTypeChanged);

    m_parameterRack = new ParameterRack(this);
    connect(m_parameterRack, &ParameterRack::parameterChanged, this, [this](int paramIdx, float val) {
        emit parameterChanged(m_index, paramIdx, val);
        requestEqCurvePublish();
    });

    m_eqChrome = new EqChrome(this);
    connect(m_eqChrome, &EqChrome::visibilityChanged, this, [this]() {
        requestEqCurvePublish(true);
    });

    m_bypassBtn = new QPushButton(tr("Bypass"), this);
    m_bypassBtn->setObjectName(Host::UI::UiObjectNames::kEffectBypassButton);
    m_bypassBtn->setCheckable(true);
    {
        QSizePolicy sp = m_bypassBtn->sizePolicy();
        sp.setRetainSizeWhenHidden(true);
        m_bypassBtn->setSizePolicy(sp);
    }
    connect(m_bypassBtn, &QPushButton::toggled, this, &EffectItem::onBypassToggled);
    setBypassVisible(false);

    m_mainLayout->addWidget(m_dragHandle);
    m_mainLayout->addWidget(m_typeCombo);
    m_mainLayout->addWidget(m_eqChrome);
    m_mainLayout->addWidget(m_parameterRack);
    m_mainLayout->addWidget(m_bypassBtn);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void EffectItem::setAvailableEffects(const QMap<int, Host::Models::EffectSpec> &availableEffects) {
    m_availableEffects = availableEffects;

    m_typeCombo->blockSignals(true);
    m_typeCombo->clear();
    for (const auto &spec : std::as_const(m_availableEffects)) {
        m_typeCombo->addItem(spec.name, spec.id);
    }
    m_typeCombo->blockSignals(false);
}

void EffectItem::setEffectChoiceEnabled(int effectTypeId, bool enabled) {
    const int row = m_typeCombo->findData(effectTypeId);
    if (row < 0) {
        return;
    }
    static_cast<QStandardItemModel*>(m_typeCombo->model())->item(row)->setEnabled(enabled);
}

void EffectItem::lockHeightForParamCapacity(int maxParamCount) {
    m_parameterRack->reserveCapacity(maxParamCount);

    setMinimumHeight(0);
    setMaximumHeight(QWIDGETSIZE_MAX);
    adjustSize();
    setFixedHeight(sizeHint().height());
}

void EffectItem::setEqCurveColorPool(EqCurveColorPool* pool) {
    if (m_eqChrome) {
        m_eqChrome->setColorPool(pool);
    }
}

void EffectItem::setBypassVisible(bool visible) {
    m_bypassBtn->setVisible(visible);
    m_bypassBtn->setEnabled(visible);
}

void EffectItem::requestEqCurvePublish(bool force) {
    if (!m_eqChrome) {
        return;
    }
    if (force || m_eqChrome->isCurveVisible()) {
        emit eqCurveDirty();
    }
}

void EffectItem::onTypeChanged(int index) {
    if (index < 0) return;
    emit typeChanged(m_index, m_typeCombo->itemData(index).toInt());
}

void EffectItem::onBypassToggled() {
    emit bypassToggled(m_index, m_bypassBtn->isChecked());
    requestEqCurvePublish();
}

void EffectItem::setupEffect(int effectTypeId) {
    m_effectTypeId = effectTypeId;

    if (!m_availableEffects.contains(effectTypeId)) {
        m_parameterRack->clearParams();
        setBypassVisible(false);
        m_eqChrome->setActive(false);
        return;
    }

    const auto &spec = m_availableEffects[effectTypeId];
    m_parameterRack->rebuild(spec);
    setBypassVisible(effectTypeId != kEmptyEffectId);
    m_eqChrome->setActive(effectTypeId == Protocol::EffectId::Eq);
}

void EffectItem::applyEffectType(int effectTypeId) {
    const int index = m_typeCombo ? m_typeCombo->findData(effectTypeId) : -1;
    if (index < 0) {
        return;
    }
    m_typeCombo->blockSignals(true);
    m_typeCombo->setCurrentIndex(index);
    m_typeCombo->blockSignals(false);
    setupEffect(effectTypeId);
}

void EffectItem::applyBypass(bool bypassed) {
    if (m_bypassBtn && m_bypassBtn->isChecked() == bypassed) {
        return;
    }
    m_bypassBtn->blockSignals(true);
    m_bypassBtn->setChecked(bypassed);
    m_bypassBtn->blockSignals(false);
    requestEqCurvePublish();
}

void EffectItem::applyParameter(int paramIndex, float value) {
    m_parameterRack->updateParam(paramIndex, value);
    requestEqCurvePublish();
}

std::optional<Host::Models::EqCurveState> EffectItem::eqCurveState() const {
    if (m_effectTypeId != Protocol::EffectId::Eq || !m_eqChrome || !m_parameterRack) {
        return std::nullopt;
    }
    const bool bypassed = m_bypassBtn && m_bypassBtn->isChecked();
    return m_eqChrome->curveState(*m_parameterRack, bypassed);
}

} // namespace Host::UI::Effects
