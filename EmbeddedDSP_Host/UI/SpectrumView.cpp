#include "SpectrumView.h"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <algorithm>
#include <cmath>

namespace {
    constexpr float MIN_FREQ = 20.0f;
    constexpr float MAX_FREQ = 20000.0f;
}

namespace UI {

SpectrumView::SpectrumView(QWidget *parent)
    : QWidget(parent) {
    setMinimumHeight(220);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAutoFillBackground(false);
}

void SpectrumView::setDbRange(float minDb, float maxDb) {
    if (maxDb <= minDb) return;
    m_minDb = minDb;
    m_maxDb = maxDb;
    update();
}

void SpectrumView::setSampleRate(float sampleRateHz) {
    m_sampleRate = sampleRateHz;
    update();
}

void SpectrumView::setFftSize(int fftSize) {
    m_fftSize = std::max(2, fftSize);
    update();
}

void SpectrumView::updateSpectrum(const std::vector<float> &magnitudeDb) {
    if (m_magnitudeDb.size() != magnitudeDb.size()) {
        m_magnitudeDb = magnitudeDb;
    } else {
        const float alpha = 0.2f;
        for (size_t i = 0; i < magnitudeDb.size(); ++i) {
            m_magnitudeDb[i] = (alpha * magnitudeDb[i]) + ((1.0f - alpha) * m_magnitudeDb[i]);
        }
    }
    update();
}

QRect SpectrumView::plotRect() const {
    constexpr int left = 48;
    constexpr int right = 12;
    constexpr int top = 12;
    constexpr int bottom = 28;
    return rect().adjusted(left, top, -right, -bottom);
}

void SpectrumView::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRect plot = plotRect();
    drawBackground(painter, plot);
    drawGrid(painter, plot);
    drawBars(painter, plot);
    drawAxesLabels(painter, plot);
}

void SpectrumView::drawBackground(QPainter &painter, const QRect &plot) const {
    painter.fillRect(rect(), m_bgColor);
    painter.fillRect(plot, QColor(18, 20, 24));
    painter.setPen(QPen(m_gridColor, 1));
    painter.drawRect(plot);
}

void SpectrumView::drawGrid(QPainter &painter, const QRect &plot) const {
    painter.setPen(QPen(m_gridColor, 1, Qt::DotLine));

    for (float db = m_minDb; db <= m_maxDb; db += 20.0f) {
        const float t = (db - m_minDb) / (m_maxDb - m_minDb);
        const int y = plot.bottom() - static_cast<int>(t * plot.height());
        painter.drawLine(plot.left(), y, plot.right(), y);
    }

    const float freqs[] = {100.0f, 1000.0f, 10000.0f};
    const float logMin = std::log10(MIN_FREQ);
    const float logMax = std::log10(MAX_FREQ);

    for (float f : freqs) {
        float t = (std::log10(f) - logMin) / (logMax - logMin);
        int x = plot.left() + static_cast<int>(t * plot.width());
        if (x > plot.left() && x < plot.right()) {
            painter.drawLine(x, plot.top(), x, plot.bottom());
        }
    }
}

void SpectrumView::drawBars(QPainter &painter, const QRect &plot) const {
    if (m_magnitudeDb.empty() || plot.width() <= 0 || plot.height() <= 0) return;

    const int binCount = static_cast<int>(m_magnitudeDb.size());
    const float binHz = (m_sampleRate * 0.5f) / static_cast<float>(binCount);
    const float dbSpan = m_maxDb - m_minDb;
    const float logMin = std::log10(MIN_FREQ);
    const float logMax = std::log10(MAX_FREQ);

    painter.setPen(QPen(m_barColor, 1.8f));
    
    QPainterPath path;
    bool first = true;

    for (int x = 0; x < plot.width(); ++x) {
        float t = static_cast<float>(x) / plot.width();
        float freq = std::pow(10.0f, logMin + t * (logMax - logMin));
        
        float binIdx = freq / binHz;
        int i0 = static_cast<int>(binIdx);
        int i1 = std::min(i0 + 1, binCount - 1);
        float frac = binIdx - static_cast<float>(i0);

        i0 = std::clamp(i0, 0, binCount - 1);
        
        float val0 = m_magnitudeDb[static_cast<std::size_t>(i0)];
        float val1 = m_magnitudeDb[static_cast<std::size_t>(i1)];
        
        float db = val0 + frac * (val1 - val0);
        db = std::clamp(db, m_minDb, m_maxDb);

        float ty = (db - m_minDb) / dbSpan;
        int y = plot.bottom() - static_cast<int>(ty * plot.height());

        if (first) {
            path.moveTo(plot.left() + x, y);
            first = false;
        } else {
            path.lineTo(plot.left() + x, y);
        }
    }
    painter.drawPath(path);
}

void SpectrumView::drawAxesLabels(QPainter &painter, const QRect &plot) const {
    painter.setPen(m_textColor);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    for (float db = m_minDb; db <= m_maxDb; db += 20.0f) {
        const float t = (db - m_minDb) / (m_maxDb - m_minDb);
        const int y = plot.bottom() - static_cast<int>(t * plot.height());
        painter.drawText(QRect(0, y - 8, plot.left() - 6, 16),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(static_cast<int>(db)));
    }

    const struct { float f; const char* lbl; } labels[] = {
        {20.0f, "20"}, {100.0f, "100"}, {1000.0f, "1k"}, {10000.0f, "10k"}, {20000.0f, "20k"}
    };
    
    const float logMin = std::log10(MIN_FREQ);
    const float logMax = std::log10(MAX_FREQ);

    for (auto const& l : labels) {
        float t = (std::log10(l.f) - logMin) / (logMax - logMin);
        int x = plot.left() + static_cast<int>(t * plot.width());
        painter.drawText(QRect(x - 20, plot.bottom() + 4, 40, 16),
                         Qt::AlignHCenter | Qt::AlignTop,
                         l.lbl);
    }
}

} // namespace UI
