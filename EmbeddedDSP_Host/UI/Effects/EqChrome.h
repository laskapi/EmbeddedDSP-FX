#ifndef EMBEDDEDDSP_HOST_EQ_CHROME_H
#define EMBEDDEDDSP_HOST_EQ_CHROME_H

#include "Models/EqCurveState.h"
#include <QWidget>
#include <optional>

namespace Host::UI::Effects {

class EqCurveButton;
class EqCurveColorPool;
class ParameterRack;

/// @brief EQ-only chrome for EffectItem: Show toggle + sticky curve color.
class EqChrome : public QWidget {
    Q_OBJECT
public:
    explicit EqChrome(QWidget* parent = nullptr);

    void setColorPool(EqCurveColorPool* pool) { m_colorPool = pool; }

    /// @brief Show/hide this chrome and manage color lifetime when slot enters/leaves EQ.
    void setActive(bool active);

    /// @brief True when the spectrum overlay should be published for this slot.
    [[nodiscard]] bool isCurveVisible() const;
    [[nodiscard]] std::optional<Host::Models::EqCurveState> curveState(
        const ParameterRack& params, bool bypassed) const;

signals:
    /// @brief Show toggle or EQ enter/leave — parent must republish (force).
    void visibilityChanged();

private slots:
    void onToggled(bool checked);

private:
    void releaseColor();

    EqCurveButton* m_button{nullptr};
    EqCurveColorPool* m_colorPool{nullptr};
    QColor m_color;
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_EQ_CHROME_H
