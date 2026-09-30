#include "SpectrumAxisX.h"
#include "SpectrumCommon.h"
#include <QPainter>
#include <QPaintEvent>
#include <QEvent>

namespace Host::UI::Spectrum {

SpectrumAxisX::SpectrumAxisX(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QSize SpectrumAxisX::sizeHint() const {
    int h = m_labelHeight * 2 + 8;
    return QSize(200, h);
}

void SpectrumAxisX::updateMetrics() {
    QFontMetrics fm(font());
    m_labelHeight = fm.height();
    
    int maxWidth = 0;
    for (const auto& l : X_TICK_LABELS) {
        maxWidth = std::max(maxWidth, fm.horizontalAdvance(l.label));
    }
    m_maxLabelWidth = maxWidth + 4;
    
    updateGeometry();
    update();
}

void SpectrumAxisX::changeEvent(QEvent* event) {
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange) {
        updateMetrics();
    }
    QWidget::changeEvent(event);
}

void SpectrumAxisX::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(palette().text().color());

    const int marginH = AXIS_SIDE_MARGIN;
    const int usableWidth = width() - (2 * marginH);
    
    for (auto const& l : X_TICK_LABELS) {
        float t = freqToPos(l.value);
        int x = marginH + static_cast<int>(t * usableWidth);
        
        painter.drawText(QRect(x - m_maxLabelWidth/2, 0, m_maxLabelWidth, m_labelHeight),
                         Qt::AlignHCenter | Qt::AlignTop,
                         l.label);
    }

    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(QColor(136, 136, 136)); 
    
    painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignBottom, "Frequency (Hz)");
}

} // namespace Host::UI::Spectrum
