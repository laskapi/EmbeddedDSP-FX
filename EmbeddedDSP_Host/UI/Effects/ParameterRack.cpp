#include "UI/Effects/ParameterRack.h"
#include "UI/Effects/ParameterItem.h"

#include <QSizePolicy>
#include <QStyle>
#include <algorithm>

namespace Host::UI::Effects {

ParameterRack::ParameterRack(QWidget *parent)
    : AbstractRack(parent) {
    layout()->setSpacing(0);
    layout()->setAlignment(Qt::AlignTop);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ParameterRack::reserveCapacity(int maxParamCount) {
    m_reservedCount = std::max(0, maxParamCount);
    const int spacing = layout()->spacing();
    const int rows = m_reservedCount;
    const int gaps = (rows > 1) ? (rows - 1) * spacing : 0;
    setFixedHeight(rows * paramRowHeight() + gaps);
}

void ParameterRack::clearParams() {
    clearRack();
    // createItem/addStretch leave non-item spacers; drop them too.
    while (layout()->count() > 0) {
        delete layout()->takeAt(0);
    }
}

void ParameterRack::rebuild(const Host::Models::EffectSpec &spec) {
    clearParams();

    for (const auto &p : spec.params) {
        auto *item = createItem(p, this);
        connect(item, &ParameterItem::valueChanged, this, &ParameterRack::parameterChanged);
    }

    // Leftover reserved height stays as empty space under the last slider.
    layout()->addStretch(1);
}

void ParameterRack::updateParam(int paramIdx, float value) {
    if (auto *item = getItem(paramIdx)) {
        item->setValue(value);
    }
}

int ParameterRack::paramRowHeight() const {
    // Matches ParameterItem: label + horizontal slider + 2px vertical margins.
    const int labelH = fontMetrics().height();
    const int sliderH = std::max(16, style()->pixelMetric(QStyle::PM_SliderThickness));
    return labelH + sliderH + 4;
}

} // namespace Host::UI::Effects
