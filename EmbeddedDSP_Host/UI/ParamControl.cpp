#include "ParamControl.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>

namespace UI {

ParamControl::ParamControl(uint8_t paramId, const Host::ParamSpec &spec, QWidget *parent)
    : QWidget(parent), m_paramId(paramId), m_spec(spec) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 2);

    m_label = new QLabel(this);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 1000);

    layout->addWidget(m_label);
    layout->addWidget(m_slider);

    connect(m_slider, &QSlider::valueChanged, this, &ParamControl::onSliderMoved);
    setValue(spec.defaultValue);
}

void ParamControl::setValue(float value) {
    m_slider->blockSignals(true);
    float range = m_spec.max - m_spec.min;
    int pos = (range > 0.0f) ? static_cast<int>((value - m_spec.min) / range * 1000.0f) : 0;
    m_slider->setValue(pos);
    updateLabel(value);
    m_slider->blockSignals(false);
}

void ParamControl::onSliderMoved(int pos) {
    float range = m_spec.max - m_spec.min;
    float value = m_spec.min + (static_cast<float>(pos) / 1000.0f) * range;
    updateLabel(value);
    emit valueChanged(m_paramId, value);
}

void ParamControl::updateLabel(float value) {
    m_label->setText(QString("%1: %2").arg(m_spec.name).arg(value, 0, 'f', 2));
}

} // namespace UI
