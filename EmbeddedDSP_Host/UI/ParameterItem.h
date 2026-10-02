#ifndef EMBEDDEDDSP_HOST_PARAMETERITEM_H
#define EMBEDDEDDSP_HOST_PARAMETERITEM_H

#include "AbstractRackItem.h"
#include "EffectSpec.h"

class QLabel;
class QSlider;

namespace Host::UI {

/**
 * @brief Individual parameter control widget (Slider + Label).
 * Inherits from AbstractRackItem to maintain its own ID for signal bubbling.
 */
class ParameterItem : public AbstractRackItem {
    Q_OBJECT
public:
    explicit ParameterItem(const ParamSpec &spec, QWidget *parent = nullptr);

    /// @brief Updates UI state from external value (e.g., from STM32).
    void setValue(float value);

signals:
    /// @brief Bubbles up the value change with the cached index.
    void valueChanged(int paramIdx, float newValue);

private slots:
    void onSliderMoved(int pos);

private:
    void updateLabel(float value);

    ParamSpec m_spec;
    float m_range{0.0f};
    QLabel *m_label{nullptr};
    QSlider *m_slider{nullptr};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_PARAMETERITEM_H
