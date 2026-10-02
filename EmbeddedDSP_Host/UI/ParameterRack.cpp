#include "ParameterRack.h"
#include "ParameterItem.h"

namespace Host::UI {

ParameterRack::ParameterRack(const EffectSpec &spec, QWidget *parent)
    : AbstractRack(parent) {
    
    for (const auto &p : spec.params) {
        auto *item = new ParameterItem(p, this);
        addItem(item);
        
        connect(item, &ParameterItem::valueChanged, this, &ParameterRack::parameterChanged);
    }
    
    layout()->addStretch(1);
}

void ParameterRack::updateParam(int paramIdx, float value) {
    if (auto *item = getItem(paramIdx)) {
        item->setValue(value);
    }
}

} // namespace Host::UI
