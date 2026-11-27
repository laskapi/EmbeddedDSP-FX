#ifndef EMBEDDEDDSP_HOST_EQ_CURVE_BUTTON_H
#define EMBEDDEDDSP_HOST_EQ_CURVE_BUTTON_H

#include <QPushButton>

namespace Host::UI::Effects {

/// @brief Checkable curve toggle that paints a sine-wave glyph.
class EqCurveButton : public QPushButton {
    Q_OBJECT
public:
    explicit EqCurveButton(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_EQ_CURVE_BUTTON_H
