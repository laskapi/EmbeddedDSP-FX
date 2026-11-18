#ifndef EMBEDDEDDSP_HOST_EFFECT_DRAG_HANDLE_H
#define EMBEDDEDDSP_HOST_EFFECT_DRAG_HANDLE_H

#include <QWidget>

namespace Host::UI::Effects {

class EffectItem;

/// @brief Top grip that starts an effect-chain drag (does not steal slider events).
class EffectDragHandle : public QWidget {
    Q_OBJECT
public:
    explicit EffectDragHandle(EffectItem* owner, QWidget* parent = nullptr);

    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    static constexpr int kHeight = 14;

    EffectItem* m_owner{nullptr};
    QPoint m_pressPos{};
    bool m_dragging{false};
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_EFFECT_DRAG_HANDLE_H
