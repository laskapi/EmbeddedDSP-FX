#include "UI/Effects/EqCurveButton.h"
#include "UI/UiTheme.h"

#include <QPainter>
#include <QPainterPath>
#include <QSizePolicy>
#include <QStyleOptionButton>
#include <cmath>

namespace Host::UI::Effects {

namespace {
constexpr int kWidth = 58;
constexpr int kHeight = 24;
constexpr qreal kPenWidth = 1.8;
constexpr int kSamples = 64;

/// @brief Flat shelves + center bell only (t in 0..1, gain ~0..1).
[[nodiscard]] qreal eqIconGain(qreal t) {
    return std::exp(-0.5 * std::pow((t - 0.50) / 0.12, 2.0));
}
}

EqCurveButton::EqCurveButton(QWidget* parent)
    : QPushButton(parent) {
    setCheckable(true);
    setText({});
    setFixedSize(kWidth, kHeight);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void EqCurveButton::paintEvent(QPaintEvent* /*event*/) {
    QStyleOptionButton opt;
    initStyleOption(&opt);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    style()->drawControl(QStyle::CE_PushButton, &opt, &painter, this);

    const QRect content = style()->subElementRect(QStyle::SE_PushButtonContents, &opt, this)
                              .adjusted(7, 3, -7, -3);
    if (content.width() < 8 || content.height() < 6) {
        return;
    }

    // Center [peak..shelf] in content: shelf at mid+amp/2, peak at mid-amp/2.
    const qreal amp = content.height() * 0.55;
    const qreal baseY = content.center().y() + amp * 0.5;

    QPainterPath curve;
    for (int i = 0; i <= kSamples; ++i) {
        const qreal t = static_cast<qreal>(i) / static_cast<qreal>(kSamples);
        const qreal x = content.left() + t * content.width();
        const qreal y = baseY - amp * eqIconGain(t);
        if (i == 0) {
            curve.moveTo(x, y);
        } else {
            curve.lineTo(x, y);
        }
    }

    const QColor waveColor = isChecked() ? QColor(QString{Host::UI::UiTheme::kWindow})
                                         : opt.palette.color(QPalette::ButtonText);
    QPen pen(waveColor, kPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(curve);
}

} // namespace Host::UI::Effects
