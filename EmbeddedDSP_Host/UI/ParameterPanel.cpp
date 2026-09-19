#include "ParameterPanel.h"
#include "ParamControl.h"
#include "StateManager.h"
#include <QVBoxLayout>

namespace UI {

ParameterPanel::ParameterPanel(const Host::EffectSpec &spec, uint8_t slotId, StateManager* manager, QWidget *parent)
    : QWidget(parent), m_manager(manager), m_slotId(slotId) {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_rack = new ControlRack(this);
    mainLayout->addWidget(m_rack);

    for (uint8_t i = 0; i < spec.params.size(); ++i) {
        auto *control = new ParamControl(i, spec.params[i], m_rack);
        m_rack->add(i, control);
        connect(control, &ParamControl::valueChanged, this, &ParameterPanel::onParamChanged);
    }
    
    m_rack->layout()->addStretch(1);
}

void ParameterPanel::updateParam(uint8_t paramId, float value) {
    if (auto *control = m_rack->get(paramId)) {
        control->setValue(value);
    }
}

void ParameterPanel::onParamChanged(uint8_t paramId, float newValue) {
    if (m_manager) {
        m_manager->setParameter(m_slotId, paramId, newValue);
    }
}

} // namespace UI
