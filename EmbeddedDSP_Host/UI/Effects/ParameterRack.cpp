#include "ParameterRack.h"
#include "ParameterItem.h"

namespace Host::UI::Effects {

ParameterRack::ParameterRack(const Host::Models::EffectSpec &spec, QWidget *parent)
    : AbstractRack(parent) {

    for (const auto &p : spec.params) {
        auto *item = createItem(p, this);

        connect(item, &ParameterItem::valueChanged, this, &ParameterRack::parameterChanged);
    }
}

void ParameterRack::updateParam(int paramIdx, float value) {
    if (auto *item = getItem(paramIdx)) {
        item->setValue(value);
    }
}

} // namespace Host::UI::Effects
