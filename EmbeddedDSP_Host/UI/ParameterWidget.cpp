#include "ParameterWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>

namespace Host::UI {

ParameterWidget::ParameterWidget(const ParamSpec &spec, QWidget *parent)
    : SynchronizedWidget(parent), m_spec(spec) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 2);

    m_label = new QLabel(this);
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 1000);

    layout->addWidget(m_label);
    layout->addWidget(m_slider);

    connect(m_slider, &QSlider::valueChanged, this, &ParameterWidget::onSliderMoved);
    setValue(spec.defaultValue);
}

void ParameterWidget::setValue(float value) {
    m_slider->blockSignals(true);
    float range = m_spec.max - m_spec.min;
    int pos = (range > 0.0f) ? static_cast<int>((value - m_spec.min) / range * 1000.0f) : 0;
    m_slider->setValue(pos);
    updateLabel(value);
    m_slider->blockSignals(false);
}

void ParameterWidget::onSliderMoved(int pos) {
    float range = m_spec.max - m_spec.min;
    float value = m_spec.min + (static_cast<float>(pos) / 1000.0f) * range;
    updateLabel(value);
    
    emit valueChanged(m_logicalIndex, value);
}

void ParameterWidget::updateLabel(float value) {
    m_label->setText(QString("%1: %2").arg(m_spec.name).arg(value, 0, 'f', 2));
}

} // namespace Host::UI
