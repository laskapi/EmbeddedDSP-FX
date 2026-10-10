#include "SpectrumPlotView.h"
#include "SpectrumCommon.h"
#include "AppController.h"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <algorithm>
#include <QStyleOption>

namespace {
    constexpr float SMOOTHING_ALPHA = 0.2f;
    constexpr float CURVE_WIDTH = 1.8f;
}

namespace Host::UI::Spectrum {

SpectrumPlotView::SpectrumPlotView(Core::AppController& controller, QWidget *parent)
    : QWidget(parent)
    , m_appController(controller) {
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAutoFillBackground(false);

    connect(&m_appController, &Core::AppController::spectrumReady, this, &SpectrumPlotView::updateSpectrum);
}

void SpectrumPlotView::setDbRange(float minDb, float maxDb) {
    if (maxDb <= minDb) return;
    m_minDb = minDb;
    m_maxDb = maxDb;
    update();
}

void SpectrumPlotView::updateSpectrum(const std::vector<float> &magnitudeDb) {
    if (m_magnitudeDb.size() != magnitudeDb.size()) {
        m_magnitudeDb = magnitudeDb;
    } else {
        for (size_t i = 0; i < magnitudeDb.size(); ++i) {
            m_magnitudeDb[i] = (SMOOTHING_ALPHA * magnitudeDb[i]) + ((1.0f - SMOOTHING_ALPHA) * m_magnitudeDb[i]);
        }
    }
    update();
}

QRect SpectrumPlotView::drawRect() const {
    const int marginH = AXIS_SIDE_MARGIN;
    const int marginV = AXIS_TOP_BOTTOM_MARGIN;
    return rect().adjusted(marginH, marginV, -marginH, -marginV);
}

void SpectrumPlotView::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const auto& pal = palette();
    const QRect plot = drawRect();

    painter.fillRect(plot, pal.alternateBase());

    drawGrid(painter, plot);
    drawCurve(painter, plot);
}

void SpectrumPlotView::drawGrid(QPainter &painter, const QRect &plot) const {
    const auto& pal = palette();
    painter.setPen(QPen(pal.color(QPalette::WindowText), 1, Qt::DotLine));

    const int dbStep = 20;
    const int numSteps = static_cast<int>(std::abs(m_minDb) / dbStep);

    for (int i = 0; i <= numSteps; ++i) {
        const float db = -static_cast<float>(i * dbStep);
        if (db < m_minDb) break;

        float t = dbToPos(db, m_minDb, m_maxDb);
        const int y = plot.bottom() - static_cast<int>(t * plot.height());
        painter.drawLine(plot.left(), y, plot.right(), y);
    }

    if (std::abs(std::fmod(m_minDb, static_cast<float>(dbStep))) > 0.5f) {
        painter.drawLine(plot.left(), plot.bottom(), plot.right(), plot.bottom());
    }

    for (const auto& l : X_TICK_LABELS) {
        float t = freqToPos(l.value);
        int x = plot.left() + static_cast<int>(t * plot.width());
        if (x > plot.left() && x < plot.right()) {
            painter.drawLine(x, plot.top(), x, plot.bottom());
        }
    }
}

void SpectrumPlotView::drawCurve(QPainter &painter, const QRect &plot) const {
    if (m_magnitudeDb.empty() || plot.width() <= 0 || plot.height() <= 0) return;

    const int binCount = static_cast<int>(m_magnitudeDb.size());
    const float binHz = (m_sampleRate * 0.5f) / static_cast<float>(binCount);

    painter.setPen(QPen(palette().highlight(), CURVE_WIDTH));

    QPainterPath path;
    bool first = true;

    for (int x = 0; x < plot.width(); ++x) {
        float t = static_cast<float>(x) / plot.width();
        float freq = std::pow(10.0f, LOG_MIN + t * (LOG_MAX - LOG_MIN));

        float binIdx = freq / binHz;
        int i0 = std::clamp(static_cast<int>(binIdx), 0, binCount - 1);

        float db = std::clamp(m_magnitudeDb[static_cast<std::size_t>(i0)], m_minDb, m_maxDb);
        float ty = dbToPos(db, m_minDb, m_maxDb);
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

} // namespace Host::UI::Spectrum
