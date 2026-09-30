#include "SpectrumAxisY.h"
#include "SpectrumCommon.h"
#include <QPainter>
#include <QPaintEvent>
#include <QEvent>

namespace Host::UI::Spectrum {

SpectrumAxisY::SpectrumAxisY(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

QSize SpectrumAxisY::sizeHint() const {
    return QSize(m_maxLabelWidth + 4, 150);
}

void SpectrumAxisY::updateMetrics() {
    QFontMetrics fm(font());
    m_labelHeight = fm.height();
    m_maxLabelWidth = fm.horizontalAdvance("-120");
    
    updateGeometry();
    update();
}

void SpectrumAxisY::changeEvent(QEvent* event) {
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange) {
        updateMetrics();
    }
    QWidget::changeEvent(event);
}

void SpectrumAxisY::setDbRange(float minDb, float maxDb) {
    if (std::abs(m_minDb - minDb) > 0.1f || std::abs(m_maxDb - maxDb) > 0.1f) {
        m_minDb = minDb;
        m_maxDb = maxDb;
        update();
    }
}

void SpectrumAxisY::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    painter.setPen(palette().text().color());

    const int dbStep = 20;
    const int numSteps = static_cast<int>(std::abs(m_minDb) / dbStep);
    const int marginV = AXIS_TOP_BOTTOM_MARGIN;
    const int usableHeight = height() - (2 * marginV);

    auto drawLabel = [&](float db) {
        float t = dbToPos(db, m_minDb, m_maxDb);
        const int y = marginV + usableHeight - static_cast<int>(t * usableHeight);
        
        painter.drawText(QRect(0, y - m_labelHeight/2, width(), m_labelHeight),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(static_cast<int>(db)));
    };

    for (int i = 0; i <= numSteps; ++i) {
        const float db = -static_cast<float>(i * dbStep);
        if (db < m_minDb) break;
        
        if (std::abs(db - m_minDb) > 8.0f) {
            drawLabel(db);
        }
    }
    drawLabel(m_minDb);
}

} // namespace Host::UI::Spectrum
