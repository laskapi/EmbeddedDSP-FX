#include "UI/Effects/EffectRack.h"
#include "UI/Effects/EffectItem.h"
#include "UI/UiObjectNames.h"
#include "Core/AppController.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QList>
#include <QMimeData>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QWidget>
#include <algorithm>

using namespace Host::Models;
using namespace Host::Core;

namespace Host::UI::Effects {

namespace {

constexpr int kArrowDepth = 18;
constexpr int kArrowBase = 55;
constexpr int kArrowGap = kArrowDepth + 8;

[[nodiscard]] bool isEffectDrag(const QMimeData* mime) {
    return mime && mime->hasFormat(QString{Host::UI::UiObjectNames::kEffectDragMimeType});
}

[[nodiscard]] int dragFromIndex(const QMimeData* mime) {
    bool ok = false;
    const int from = mime->data(QString{Host::UI::UiObjectNames::kEffectDragMimeType}).toInt(&ok);
    return ok ? from : -1;
}

void drawChainArrows(QPainter& painter,
                     const QList<EffectItem*>& items,
                     int rackHeight,
                     const QColor& color) {
    if (items.size() < 2) {
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);

    for (int i = 0; i < items.size() - 1; ++i) {
        const EffectItem* leftItem = items[i];
        const EffectItem* rightItem = items[i + 1];
        if (!leftItem || !rightItem) {
            continue;
        }

        const qreal gapLeft = leftItem->geometry().right();
        const qreal gapRight = rightItem->geometry().left();
        const qreal left = (gapLeft + gapRight - kArrowDepth) * 0.5;
        const qreal cy = rackHeight * 0.5;

        QPainterPath tri;
        tri.moveTo(left + kArrowDepth, cy);
        tri.lineTo(left, cy - kArrowBase * 0.5);
        tri.lineTo(left, cy + kArrowBase * 0.5);
        tri.closeSubpath();
        painter.drawPath(tri);
    }
}

} // namespace

EffectRack::EffectRack(AppController& controller, QWidget *parent)
    : AbstractRack(parent), m_appController(controller) {
    layout()->setSpacing(kArrowGap);
    setAcceptDrops(true);

    m_dropIndicator = new QWidget(this);
    m_dropIndicator->setObjectName(Host::UI::UiObjectNames::kEffectDropIndicator);
    m_dropIndicator->setAttribute(Qt::WA_StyledBackground, true);
    m_dropIndicator->setFixedWidth(3);
    m_dropIndicator->hide();
    m_dropIndicator->setAttribute(Qt::WA_TransparentForMouseEvents);

    connect(&m_appController, &AppController::manifestReady, this, &EffectRack::onManifestReady);
    connect(&m_appController, &AppController::slotParameterUpdated, this, &EffectRack::onSlotParameterUpdated);
    connect(&m_appController, &AppController::slotEffectTypeUpdated, this, &EffectRack::onSlotEffectTypeUpdated);
    connect(&m_appController, &AppController::slotBypassUpdated, this, &EffectRack::onSlotBypassUpdated);
}

void EffectRack::onManifestReady(const DeviceManifest& manifest) {
    clear();
    m_slotsToInit = manifest.slotCount;

    int maxParamCount = 0;
    for (const auto& spec : manifest.availableEffects) {
        maxParamCount = std::max(maxParamCount, static_cast<int>(spec.params.size()));
    }

    for (int i = 0; i < manifest.slotCount; ++i) {
        auto *item = createItem(this);
        item->setAvailableEffects(manifest.availableEffects);
        item->lockHeightForParamCapacity(maxParamCount);

        item->setEqCurveColorPool(&m_eqCurveColors);
        connect(item, &EffectItem::parameterChanged, &m_appController, &AppController::setParameter);
        connect(item, &EffectItem::typeChanged, &m_appController, &AppController::setEffectType);
        connect(item, &EffectItem::bypassToggled, &m_appController, &AppController::setBypass);
        connect(item, &EffectItem::eqCurveDirty, this, &EffectRack::publishEqCurves);
    }
    syncItemLayout();
    publishEqCurves();
}

void EffectRack::clear() {
    hideDropIndicator();
    while (QLayoutItem* child = layout()->takeAt(0)) {
        delete child;
    }
    clearRack();
    m_eqCurveColors.reset();
    m_slotsToInit = -1;
    m_delayOwner = -1;
    publishEqCurves();
}

void EffectRack::publishEqCurves() {
    QVector<Host::Models::EqCurveState> curves;
    curves.reserve(itemCount());
    for (int i = 0; i < itemCount(); ++i) {
        if (EffectItem* item = getItem(i)) {
            if (auto state = item->eqCurveState()) {
                curves.push_back(*state);
            }
        }
    }
    emit eqCurvesChanged(curves);
}

void EffectRack::syncItemLayout() {
    while (QLayoutItem* li = layout()->takeAt(0)) {
        delete li;
    }

    for (int i = 0; i < itemCount(); ++i) {
        EffectItem* item = getItem(i);
        layout()->addWidget(item, 1);
    }
    update();
}

void EffectRack::optimisticMove(int from, int to) {
    moveItem(from, to);
    syncItemLayout();
    publishEqCurves();
}

void EffectRack::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    QList<EffectItem*> items;
    items.reserve(itemCount());
    for (int i = 0; i < itemCount(); ++i) {
        items.append(getItem(i));
    }

    QPainter p(this);
    drawChainArrows(p, items, height(), palette().color(QPalette::Highlight));
}

int EffectRack::insertIndexFor(int x) const {
    for (int i = 0; i < itemCount(); ++i) {
        const EffectItem* item = getItem(i);
        if (!item) {
            continue;
        }
        if (x < item->geometry().center().x()) {
            return i;
        }
    }
    return itemCount();
}

void EffectRack::showDropIndicator(int insertBefore) {
    if (itemCount() == 0) {
        hideDropIndicator();
        return;
    }

    int x = 0;
    if (insertBefore <= 0) {
        x = getItem(0)->geometry().left();
    } else if (insertBefore >= itemCount()) {
        x = getItem(itemCount() - 1)->geometry().right() - m_dropIndicator->width();
    } else {
        const int lowerRight = getItem(insertBefore - 1)->geometry().right();
        const int higherLeft = getItem(insertBefore)->geometry().left();
        x = (lowerRight + higherLeft - m_dropIndicator->width()) / 2;
    }

    m_dropIndicator->setGeometry(x, 0, m_dropIndicator->width(), height());
    m_dropIndicator->show();
    m_dropIndicator->raise();
}

void EffectRack::hideDropIndicator() {
    m_dropIndicator->hide();
}

void EffectRack::dragEnterEvent(QDragEnterEvent* event) {
    if (isEffectDrag(event->mimeData())) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void EffectRack::dragMoveEvent(QDragMoveEvent* event) {
    if (!isEffectDrag(event->mimeData())) {
        event->ignore();
        return;
    }
    showDropIndicator(insertIndexFor(event->position().toPoint().x()));
    event->acceptProposedAction();
}

void EffectRack::dragLeaveEvent(QDragLeaveEvent* event) {
    hideDropIndicator();
    QWidget::dragLeaveEvent(event);
}

void EffectRack::dropEvent(QDropEvent* event) {
    hideDropIndicator();
    if (!isEffectDrag(event->mimeData())) {
        event->ignore();
        return;
    }

    const int from = dragFromIndex(event->mimeData());
    int insertBefore = insertIndexFor(event->position().toPoint().x());
    if (from < 0 || from >= itemCount()) {
        event->ignore();
        return;
    }

    int to = insertBefore;
    if (from < insertBefore) {
        --to;
    }
    if (to < 0 || to >= itemCount() || to == from) {
        event->ignore();
        return;
    }

    optimisticMove(from, to);
    m_appController.moveEffect(from, to);
    event->acceptProposedAction();
}

void EffectRack::onSlotParameterUpdated(int slotIndex, int paramIndex, float value) {
    if (auto *item = getItem(slotIndex)) {
        item->applyParameter(paramIndex, value);
    }
}

void EffectRack::onSlotEffectTypeUpdated(int slotIndex, int effectTypeId) {
    if (auto *item = getItem(slotIndex)) {
        item->applyEffectType(effectTypeId);

        if (effectTypeId == Protocol::EffectId::Delay) {
            m_delayOwner = slotIndex;
        } else if (m_delayOwner == slotIndex) {
            m_delayOwner = -1;
        }
        refreshDelayExclusivity();

        if (m_slotsToInit > 0 && --m_slotsToInit == 0) {
            emit rackReady();
        }
        // Type change may show/hide EQ curve controls.
        publishEqCurves();
    }
}

void EffectRack::refreshDelayExclusivity() {
    for (int i = 0; i < itemCount(); ++i) {
        getItem(i)->setEffectChoiceEnabled(Protocol::EffectId::Delay,
                                           m_delayOwner < 0 || m_delayOwner == i);
    }
}

void EffectRack::onSlotBypassUpdated(int slotIndex, bool bypassed) {
    if (auto *item = getItem(slotIndex)) {
        item->applyBypass(bypassed);
    }
}

} // namespace Host::UI::Effects
