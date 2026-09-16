#include "EffectModule.h"
#include "ParamControl.h"
#include <QVBoxLayout>

namespace UI {

EffectModule::EffectModule(const Host::EffectSpec &spec, QWidget *parent)
    : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    for (uint8_t i = 0; i < spec.params.size(); ++i) {
        auto *control = new ParamControl(i, spec.params[i], this);
        m_controls[i] = control;
        connect(control, &ParamControl::valueChanged, this, &EffectModule::paramChanged);
        layout->addWidget(control);
    }
    layout->addStretch(1);
}

void EffectModule::updateParam(uint8_t paramId, float value) {
    if (m_controls.contains(paramId)) {
        m_controls[paramId]->setValue(value);
    }
}

} // namespace UI
