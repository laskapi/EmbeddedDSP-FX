#ifndef EMBEDDEDDSP_HOST_PARAMCONTROL_H
#define EMBEDDEDDSP_HOST_PARAMCONTROL_H

#include <QWidget>
#include "EffectSpec.h"

class QLabel;
class QSlider;

namespace Host::UI {

/// @brief Individual parameter widget consisting of a label and a slider.
class ParamControl : public QWidget {
    Q_OBJECT
public:
    explicit ParamControl(uint8_t paramId, const ParamSpec &spec, QWidget *parent = nullptr);

    /// @brief Sets the slider position from a normalized value.
    void setValue(float value);

signals:
    /// @brief Emitted when the user modifies the parameter.
    void valueChanged(uint8_t paramId, float newValue);

private slots:
    void onSliderMoved(int pos);

private:
    void updateLabel(float value);

    uint8_t m_paramId;
    ParamSpec m_spec;
    QLabel *m_label{nullptr};
    QSlider *m_slider{nullptr};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_PARAMCONTROL_H
