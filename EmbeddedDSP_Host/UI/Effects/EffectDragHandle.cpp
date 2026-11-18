#include "UI/Effects/EffectDragHandle.h"
#include "UI/Effects/EffectItem.h"
#include "UI/UiObjectNames.h"

#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>

namespace Host::UI::Effects {

EffectDragHandle::EffectDragHandle(EffectItem* owner, QWidget* parent)
    : QWidget(parent)
    , m_owner(owner)
{
    setObjectName(Host::UI::UiObjectNames::kEffectDragHandle);
    setCursor(Qt::OpenHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(kHeight);
}

QSize EffectDragHandle::sizeHint() const {
    return {0, kHeight};
}

void EffectDragHandle::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(palette().color(QPalette::Highlight));

    const int cy = height() / 2;
    const int cx = width() / 2;
    constexpr int kSpread = 5;
    for (int row = -1; row <= 1; row += 2) {
        for (int col = -1; col <= 1; ++col) {
            p.drawEllipse(QPointF(cx + col * kSpread, cy + row * 2.5), 1.6, 1.6);
        }
    }
}

void EffectDragHandle::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_pressPos = event->pos();
        m_dragging = false;
        setCursor(Qt::ClosedHandCursor);
    }
    QWidget::mousePressEvent(event);
}

void EffectDragHandle::mouseReleaseEvent(QMouseEvent* event) {
    setCursor(Qt::OpenHandCursor);
    m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}

void EffectDragHandle::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton) || m_dragging || !m_owner) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    if ((event->pos() - m_pressPos).manhattanLength() < QApplication::startDragDistance()) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    m_dragging = true;
    auto* mime = new QMimeData();
    mime->setData(QString{Host::UI::UiObjectNames::kEffectDragMimeType},
                  QByteArray::number(m_owner->slotIndex()));

    auto* drag = new QDrag(this);
    drag->setMimeData(mime);
    const QPixmap ghost = m_owner->grab();
    drag->setPixmap(ghost);
    drag->setHotSpot(QPoint(ghost.width() / 2, 8));
    drag->exec(Qt::MoveAction);
    setCursor(Qt::OpenHandCursor);
    m_dragging = false;
}

} // namespace Host::UI::Effects
