#include "ParameterItem.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>

namespace {
    constexpr int SLIDER_RESOLUTION = 1000;
}

namespace Host::UI::Effects {

ParameterItem::ParameterItem(int index, const Host::Models::ParamSpec &spec, QWidget *parent)
    : AbstractRackItem(index, parent), m_spec(spec) {
    m_range = m_spec.max - m_spec.min;

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 2);
    layout->setSpacing(0);

    m_label = new QLabel(this);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, SLIDER_RESOLUTION);

    layout->addWidget(m_label);
    layout->addWidget(m_slider);

    connect(m_slider, &QSlider::valueChanged, this, &ParameterItem::onSliderMoved);
    setValue(spec.defaultValue);
}

void ParameterItem::setValue(float value) {
    m_slider->blockSignals(true);
    int pos = (m_range > 0.0f) ? static_cast<int>((value - m_spec.min) / m_range * SLIDER_RESOLUTION) : 0;
    m_slider->setValue(pos);
    updateLabel(value);
    m_slider->blockSignals(false);
}

void ParameterItem::onSliderMoved(int pos) {
    float value = m_spec.min + (static_cast<float>(pos) / SLIDER_RESOLUTION) * m_range;
    updateLabel(value);

    emit valueChanged(m_index, value);
}

void ParameterItem::updateLabel(float value) {
    m_label->setText(QString("%1: %2").arg(m_spec.name).arg(value, 0, 'f', 2));
}

} // namespace Host::UI::Effects
