#ifndef EMBEDDEDDSP_HOST_PARAMCONTROL_H
#define EMBEDDEDDSP_HOST_PARAMCONTROL_H

#include <QWidget>
#include "EffectSpec.h"

class QLabel;
class QSlider;

namespace UI {

/**
 * @brief Individual parameter control widget (Label + Slider).
 */
class ParamControl : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief Constructor for ParamControl.
     * @param paramId Local ID of the parameter.
     * @param spec Specification (name, range, default).
     * @param parent Pointer to the parent QObject.
     */
    explicit ParamControl(uint8_t paramId, const Host::ParamSpec &spec, QWidget *parent = nullptr);

    /** @brief Sets the current slider position based on normalized float value. */
    void setValue(float value);

signals:
    /** @brief Emitted when the user moves the slider. */
    void valueChanged(uint8_t paramId, float newValue);

private slots:
    void onSliderMoved(int pos);

private:
    void updateLabel(float value);

    uint8_t m_paramId;
    Host::ParamSpec m_spec;
    QLabel *m_label{nullptr};
    QSlider *m_slider{nullptr};
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_PARAMCONTROL_H
