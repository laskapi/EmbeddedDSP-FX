#ifndef EMBEDDEDDSP_HOST_EFFECT_CHAIN_ARROW_H
#define EMBEDDEDDSP_HOST_EFFECT_CHAIN_ARROW_H

#include <QWidget>

namespace Host::UI::Effects {

/// @brief Painted L→R chain separator.
class EffectChainArrow : public QWidget {
    Q_OBJECT
public:
    explicit EffectChainArrow(QWidget* parent = nullptr);

    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_EFFECT_CHAIN_ARROW_H
