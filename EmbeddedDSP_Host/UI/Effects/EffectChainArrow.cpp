#include "UI/Effects/EffectChainArrow.h"
#include "UI/UiObjectNames.h"

#include <QPainter>
#include <QPainterPath>
#include <QSizePolicy>

namespace Host::UI::Effects {

namespace {

constexpr int kBase = 55;  // vertical side
constexpr int kDepth = 18; // tip → base
constexpr int kPad = 4;    
constexpr int kWidgetWidth = kDepth + 2 * kPad;
constexpr QColor kAccent{0x50, 0xc8, 0x8c};

} // namespace

EffectChainArrow::EffectChainArrow(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(Host::UI::UiObjectNames::kEffectChainArrow);
    setFixedWidth(kWidgetWidth);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

QSize EffectChainArrow::sizeHint() const {
    return {kWidgetWidth, kBase + 2 * kPad};
}

void EffectChainArrow::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Gap between slots == this widget; triangle inset equally L/R.
    const qreal left = (width() - kDepth) * 0.5;
    const qreal cy = height() * 0.5;
    const QPointF tip(left + kDepth, cy);
    const QPointF top(left, cy - kBase * 0.5);
    const QPointF bottom(left, cy + kBase * 0.5);

    QPainterPath tri;
    tri.moveTo(tip);
    tri.lineTo(top);
    tri.lineTo(bottom);
    tri.closeSubpath();

    p.setPen(Qt::NoPen);
    p.setBrush(kAccent);
    p.drawPath(tri);
}

} // namespace Host::UI::Effects
