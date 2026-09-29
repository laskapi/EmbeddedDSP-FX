#include "ParameterRack.h"

namespace Host::UI {

ParameterRack::ParameterRack(const EffectSpec &spec, QWidget *parent)
    : SynchronizedRack(parent) {
    
    for (const auto &p : spec.params) {
        auto *control = new ParameterWidget(p, this);
        addSynchronized(control);
        
        connect(control, &ParameterWidget::valueChanged, this, &ParameterRack::parameterChanged);
    }
    
    layout()->addStretch(1);
}

void ParameterRack::updateParam(int paramIdx, float value) {
    if (auto *control = getSynchronized(paramIdx)) {
        control->setValue(value);
    }
}

} // namespace Host::UI
