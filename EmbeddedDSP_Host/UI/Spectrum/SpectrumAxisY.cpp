#include "UI/Spectrum/SpectrumAxisY.h"
#include "UI/Spectrum/SpectrumCommon.h"
#include <QPainter>
#include <QPaintEvent>
#include <QEvent>
#include <cmath>

namespace Host::UI::Spectrum {

SpectrumAxisY::SpectrumAxisY(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    ensurePolished();
    updateMetrics();
}

QSize SpectrumAxisY::sizeHint() const {
    return QSize(m_maxLabelWidth + 4, 150);
}

QSize SpectrumAxisY::minimumSizeHint() const {
    return sizeHint();
}

void SpectrumAxisY::updateMetrics() {
    QFontMetrics fm(font());
    m_labelHeight = fm.height();
    m_maxLabelWidth = fm.horizontalAdvance(QStringLiteral("-120"));
    setFixedWidth(m_maxLabelWidth + 4);

    updateGeometry();
    update();
}

void SpectrumAxisY::changeEvent(QEvent* event) {
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange) {
        updateMetrics();
    }
    QWidget::changeEvent(event);
}

void SpectrumAxisY::setFftDbRange(float minDb, float maxDb) {
    if (maxDb <= minDb) {
        return;
    }
    if (std::abs(m_fftMinDb - minDb) > 0.1f || std::abs(m_fftMaxDb - maxDb) > 0.1f) {
        m_fftMinDb = minDb;
        m_fftMaxDb = maxDb;
        update();
    }
}

void SpectrumAxisY::setEqMode(bool eqMode) {
    if (m_eqMode == eqMode) {
        return;
    }
    m_eqMode = eqMode;
    update();
}

void SpectrumAxisY::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int marginV = AXIS_TOP_BOTTOM_MARGIN;
    const int usableHeight = height() - (2 * marginV);

    if (m_eqMode) {
        drawEqTicks(painter, marginV, usableHeight);
    } else {
        drawFftTicks(painter, marginV, usableHeight);
    }
}

void SpectrumAxisY::drawFftTicks(QPainter& painter, int marginV, int usableHeight) const {
    painter.setPen(palette().text().color());

    auto drawLabel = [&](float db) {
        const float t = dbToPos(db, m_fftMinDb, m_fftMaxDb);
        const int y = marginV + usableHeight - static_cast<int>(t * usableHeight);
        painter.drawText(QRect(0, y - m_labelHeight / 2, width(), m_labelHeight),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(static_cast<int>(db)));
    };

    const int dbStep = 20;
    const int numSteps = static_cast<int>(std::abs(m_fftMinDb) / dbStep);
    for (int i = 0; i <= numSteps; ++i) {
        const float db = -static_cast<float>(i * dbStep);
        if (db < m_fftMinDb) {
            break;
        }
        if (std::abs(db - m_fftMinDb) > 8.0f) {
            drawLabel(db);
        }
    }
    drawLabel(m_fftMinDb);
}

void SpectrumAxisY::drawEqTicks(QPainter& painter, int marginV, int usableHeight) const {
    painter.setPen(palette().text().color());

    constexpr int kStep = 6;
    for (int db = static_cast<int>(std::ceil(EQ_MIN_DB / kStep) * kStep);
         db <= static_cast<int>(EQ_MAX_DB); db += kStep) {
        const float t = dbToPos(static_cast<float>(db), EQ_MIN_DB, EQ_MAX_DB);
        const int y = marginV + usableHeight - static_cast<int>(t * usableHeight);
        painter.drawText(QRect(0, y - m_labelHeight / 2, width(), m_labelHeight),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(db));
    }
}

} // namespace Host::UI::Spectrum
