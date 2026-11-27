#include "UI/Effects/EqChrome.h"
#include "UI/Effects/EqCurveButton.h"
#include "UI/Effects/EqCurveColorPool.h"
#include "UI/Effects/ParameterRack.h"
#include "UI/UiObjectNames.h"

#include <QHBoxLayout>
#include <QSizePolicy>

namespace Host::UI::Effects {

EqChrome::EqChrome(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_button = new EqCurveButton(this);
    m_button->setObjectName(Host::UI::UiObjectNames::kEffectEqCurveButton);
    {
        QSizePolicy sp = sizePolicy();
        sp.setRetainSizeWhenHidden(true);
        sp.setHorizontalPolicy(QSizePolicy::Preferred);
        setSizePolicy(sp);
    }
    setMinimumHeight(26);
    connect(m_button, &QPushButton::toggled, this, &EqChrome::onToggled);

    layout->addStretch(1);
    layout->addWidget(m_button, 0, Qt::AlignRight | Qt::AlignVCenter);
    setActive(false);
}

void EqChrome::setActive(bool active) {
    if (active) {
        if (!m_color.isValid() && m_colorPool) {
            m_color = m_colorPool->acquire();
        }
        setVisible(true);
        setEnabled(true);
        m_button->setVisible(true);
        m_button->setEnabled(true);
        onToggled(m_button->isChecked());
    } else {
        if (m_button->isChecked()) {
            m_button->blockSignals(true);
            m_button->setChecked(false);
            m_button->blockSignals(false);
        }
        releaseColor();
        setVisible(false);
        setEnabled(false);
        m_button->setVisible(false);
        m_button->setEnabled(false);
        onToggled(false);
    }
}

bool EqChrome::isCurveVisible() const {
    return m_button && m_button->isVisible() && m_button->isChecked();
}

std::optional<Host::Models::EqCurveState> EqChrome::curveState(const ParameterRack& params,
                                                              bool bypassed) const {
    if (!isCurveVisible() || bypassed || !m_color.isValid() || params.paramCount() < 5) {
        return std::nullopt;
    }

    Host::Models::EqCurveState state;
    state.color = m_color;
    state.bassDb = params.paramValue(0);
    state.midDb = params.paramValue(1);
    state.trebleDb = params.paramValue(2);
    state.midHz = params.paramValue(3);
    state.midQ = params.paramValue(4);
    return state;
}

void EqChrome::onToggled(bool checked) {
    if (!m_button) {
        return;
    }

    m_button->setToolTip(checked ? tr("Hide EQ curve") : tr("Show EQ curve"));

    if (checked && m_color.isValid()) {
        m_button->setStyleSheet(
            QStringLiteral("QPushButton:checked{background-color:%1;border:1px solid %1;}")
                .arg(m_color.name()));
    } else {
        m_button->setStyleSheet({});
    }

    emit visibilityChanged();
}

void EqChrome::releaseColor() {
    if (m_colorPool && m_color.isValid()) {
        m_colorPool->release(m_color);
    }
    m_color = QColor();
}

} // namespace Host::UI::Effects
