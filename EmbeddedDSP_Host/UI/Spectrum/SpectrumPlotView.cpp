#include "UI/Spectrum/SpectrumPlotView.h"
#include "UI/Spectrum/SpectrumCommon.h"
#include "Core/AppController.h"
#include "Dsp/EqToneStack.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <algorithm>

namespace {
constexpr float kSmoothingAlpha = 0.2f;
constexpr float kCurveWidth = 1.8f;
constexpr float kEqCurveWidth = 2.2f;
constexpr qreal kFftDimOpacity = 0.22;
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

void SpectrumPlotView::setEqCurves(const QVector<Host::Models::EqCurveState>& curves) {
    m_eqCurves = curves;
    update();
}

void SpectrumPlotView::updateSpectrum(const std::vector<float> &magnitudeDb) {
    if (m_magnitudeDb.size() != magnitudeDb.size()) {
        m_magnitudeDb = magnitudeDb;
    } else {
        for (size_t i = 0; i < magnitudeDb.size(); ++i) {
            m_magnitudeDb[i] = (kSmoothingAlpha * magnitudeDb[i]) + ((1.0f - kSmoothingAlpha) * m_magnitudeDb[i]);
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

    const bool eqMode = !m_eqCurves.isEmpty();
    painter.setOpacity(eqMode ? kFftDimOpacity : 1.0);
    drawCurve(painter, plot);
    painter.setOpacity(1.0);

    if (eqMode) {
        drawEqZeroLine(painter, plot);
        drawEqCurves(painter, plot);
    }
}

void SpectrumPlotView::drawGrid(QPainter &painter, const QRect &plot) const {
    const auto& pal = palette();
    painter.setPen(QPen(pal.color(QPalette::WindowText), 1, Qt::DotLine));

    if (!m_eqCurves.isEmpty()) {
        for (int db = static_cast<int>(EQ_MIN_DB); db <= static_cast<int>(EQ_MAX_DB); db += 6) {
            float t = dbToPos(static_cast<float>(db), EQ_MIN_DB, EQ_MAX_DB);
            const int y = plot.bottom() - static_cast<int>(t * plot.height());
            painter.drawLine(plot.left(), y, plot.right(), y);
        }
    } else {
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

    painter.setPen(QPen(palette().highlight(), kCurveWidth));

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

void SpectrumPlotView::drawEqZeroLine(QPainter &painter, const QRect &plot) const {
    const float t = dbToPos(0.0f, EQ_MIN_DB, EQ_MAX_DB);
    const int y = plot.bottom() - static_cast<int>(t * plot.height());
    QColor zero = palette().color(QPalette::WindowText);
    zero.setAlpha(120);
    painter.setPen(QPen(zero, 1, Qt::DashLine));
    painter.drawLine(plot.left(), y, plot.right(), y);
}

void SpectrumPlotView::drawEqCurves(QPainter &painter, const QRect &plot) const {
    if (plot.width() <= 0 || plot.height() <= 0) {
        return;
    }

    for (const Host::Models::EqCurveState& curve : m_eqCurves) {
        painter.setPen(QPen(curve.color, kEqCurveWidth));
        QPainterPath path;
        bool first = true;

        for (int x = 0; x < plot.width(); ++x) {
            const float t = static_cast<float>(x) / static_cast<float>(plot.width());
            const float freq = std::pow(10.0f, LOG_MIN + t * (LOG_MAX - LOG_MIN));
            const float db = Shared::Dsp::eqCascadeMagnitudeDb(
                m_sampleRate, curve.bassDb, curve.midDb, curve.trebleDb, curve.midHz, curve.midQ, freq);
            const float ty = dbToPos(db, EQ_MIN_DB, EQ_MAX_DB);
            const int y = plot.bottom() - static_cast<int>(ty * plot.height());

            if (first) {
                path.moveTo(plot.left() + x, y);
                first = false;
            } else {
                path.lineTo(plot.left() + x, y);
            }
        }
        painter.drawPath(path);
    }
}

} // namespace Host::UI::Spectrum
