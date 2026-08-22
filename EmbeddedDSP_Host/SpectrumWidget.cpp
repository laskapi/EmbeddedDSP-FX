#include "SpectrumWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <algorithm>
#include <cmath>

SpectrumWidget::SpectrumWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(220);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAutoFillBackground(false);
}

void SpectrumWidget::setDbRange(float minDb, float maxDb)
{
    if (maxDb <= minDb) {
        return;
    }
    m_minDb = minDb;
    m_maxDb = maxDb;
    update();
}

void SpectrumWidget::setSampleRate(float sampleRateHz)
{
    m_sampleRate = sampleRateHz;
    update();
}

void SpectrumWidget::setFftSize(int fftSize)
{
    m_fftSize = std::max(2, fftSize);
    update();
}

void SpectrumWidget::updateSpectrum(const std::vector<float> &magnitudeDb)
{
    m_magnitudeDb = magnitudeDb;
    update();
}

QRect SpectrumWidget::plotRect() const
{
    constexpr int left = 48;
    constexpr int right = 12;
    constexpr int top = 12;
    constexpr int bottom = 28;
    return rect().adjusted(left, top, -right, -bottom);
}

void SpectrumWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRect plot = plotRect();
    drawBackground(painter, plot);
    drawGrid(painter, plot);
    drawBars(painter, plot);
    drawAxesLabels(painter, plot);
}

void SpectrumWidget::drawBackground(QPainter &painter, const QRect &plot) const
{
    painter.fillRect(rect(), m_bgColor);
    painter.fillRect(plot, QColor(18, 20, 24));
    painter.setPen(QPen(m_gridColor, 1));
    painter.drawRect(plot);
}

void SpectrumWidget::drawGrid(QPainter &painter, const QRect &plot) const
{
    painter.setPen(QPen(m_gridColor, 1, Qt::DotLine));

    // Horizontal dB lines every 20 dB
    for (float db = m_minDb; db <= m_maxDb; db += 20.0f) {
        const float t = (db - m_minDb) / (m_maxDb - m_minDb);
        const int y = plot.bottom() - static_cast<int>(t * plot.height());
        painter.drawLine(plot.left(), y, plot.right(), y);
    }

    // Vertical frequency guides (4 divisions)
    for (int i = 1; i < 4; ++i) {
        const int x = plot.left() + (plot.width() * i) / 4;
        painter.drawLine(x, plot.top(), x, plot.bottom());
    }
}

void SpectrumWidget::drawBars(QPainter &painter, const QRect &plot) const
{
    if (m_magnitudeDb.empty() || plot.width() <= 0 || plot.height() <= 0) {
        return;
    }

    const int binCount = static_cast<int>(m_magnitudeDb.size());
    const float barWidth = static_cast<float>(plot.width()) / static_cast<float>(binCount);
    const float dbSpan = m_maxDb - m_minDb;

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_barColor);

    for (int i = 0; i < binCount; ++i) {
        const float db = std::clamp(m_magnitudeDb[static_cast<std::size_t>(i)], m_minDb, m_maxDb);
        const float t = (db - m_minDb) / dbSpan;
        const int barHeight = std::max(1, static_cast<int>(t * plot.height()));
        const int x = plot.left() + static_cast<int>(i * barWidth);
        const int w = std::max(1, static_cast<int>(barWidth) - 1);
        const int y = plot.bottom() - barHeight + 1;

        painter.drawRect(x, y, w, barHeight);
    }
}

void SpectrumWidget::drawAxesLabels(QPainter &painter, const QRect &plot) const
{
    painter.setPen(m_textColor);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    // dB labels
    for (float db = m_minDb; db <= m_maxDb; db += 20.0f) {
        const float t = (db - m_minDb) / (m_maxDb - m_minDb);
        const int y = plot.bottom() - static_cast<int>(t * plot.height());
        painter.drawText(QRect(0, y - 8, plot.left() - 6, 16),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QStringLiteral("%1").arg(static_cast<int>(db)));
    }

    // Frequency labels (0 .. Nyquist)
    const float nyquist = m_sampleRate * 0.5f;
    for (int i = 0; i <= 4; ++i) {
        const float hz = nyquist * static_cast<float>(i) / 4.0f;
        const int x = plot.left() + (plot.width() * i) / 4;
        QString label;
        if (hz >= 1000.0f) {
            label = QStringLiteral("%1k").arg(hz / 1000.0f, 0, 'f', 1);
        } else {
            label = QStringLiteral("%1").arg(static_cast<int>(hz));
        }
        painter.drawText(QRect(x - 24, plot.bottom() + 4, 48, 16),
                         Qt::AlignHCenter | Qt::AlignTop,
                         label);
    }
}