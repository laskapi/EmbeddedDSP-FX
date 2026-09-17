#include "EffectModule.h"
#include "ParamControl.h"
#include <QVBoxLayout>

namespace UI {

EffectModule::EffectModule(const Host::EffectSpec &spec, QWidget *parent)
    : QWidget(parent) {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_rack = new ControlRack(this);
    mainLayout->addWidget(m_rack);

    for (uint8_t i = 0; i < spec.params.size(); ++i) {
        auto *control = new ParamControl(i, spec.params[i], m_rack);
        m_rack->add(i, control);
        connect(control, &ParamControl::valueChanged, this, &EffectModule::paramChanged);
    }
    
    m_rack->layout()->addStretch(1);
}

void EffectModule::updateParam(uint8_t paramId, float value) {
    if (auto *control = m_rack->get(paramId)) {
        control->setValue(value);
    }
}

} // namespace UI
