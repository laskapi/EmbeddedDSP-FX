#include "SpectrumView.h"
#include "AppController.h"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <algorithm>
#include <cmath>

namespace {
    constexpr float MIN_FREQ = 20.0f;
    constexpr float MAX_FREQ = 20000.0f;
    const float LOG_MIN = std::log10(MIN_FREQ);
    const float LOG_MAX = std::log10(MAX_FREQ);
    
    // Aesthetic constants
    constexpr float SMOOTHING_ALPHA = 0.2f;
    constexpr float CURVE_WIDTH = 1.8f;
    constexpr int AXIS_LABEL_FONT_SIZE = 8;
}

namespace Host::UI {

SpectrumView::SpectrumView(std::shared_ptr<AppController> controller, QWidget *parent)
    : QWidget(parent)
    , m_appController(controller) {
    Q_ASSERT(m_appController);
    setObjectName("SpectrumView");
    setMinimumHeight(180);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    setAttribute(Qt::WA_StyledBackground);
    setAutoFillBackground(false);

    connect(m_appController.get(), &AppController::spectrumReady, this, &SpectrumView::updateSpectrum);
}

void SpectrumView::setDbRange(float minDb, float maxDb) {
    if (maxDb <= minDb) return;
    m_minDb = minDb;
    m_maxDb = maxDb;
    update();
}

void SpectrumView::updateSpectrum(const std::vector<float> &magnitudeDb) {
    if (m_magnitudeDb.size() != magnitudeDb.size()) {
        m_magnitudeDb = magnitudeDb;
    } else {
        for (size_t i = 0; i < magnitudeDb.size(); ++i) {
            m_magnitudeDb[i] = (SMOOTHING_ALPHA * magnitudeDb[i]) + ((1.0f - SMOOTHING_ALPHA) * m_magnitudeDb[i]);
        }
    }
    update();
}

QRect SpectrumView::plotRect() const {
    constexpr int left = 44; 
    constexpr int right = 44; 
    constexpr int top = 8;
    return rect().adjusted(left, top, -right, -AXIS_BOTTOM_MARGIN);
}

void SpectrumView::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRect plot = plotRect();
    drawBackground(painter, plot);
    drawGrid(painter, plot);
    drawCurve(painter, plot);
    drawAxesLabels(painter, plot);
}

void SpectrumView::drawBackground(QPainter &painter, const QRect &plot) const {
    const auto& pal = palette();
    
    /// @brief Inner plot area.
    painter.fillRect(plot, pal.alternateBase());
    
    /// @brief Border around the plot area.
    painter.setPen(QPen(pal.color(QPalette::Active, QPalette::ToolTipBase), 1));
    painter.drawRect(plot);
}

void SpectrumView::drawGrid(QPainter &painter, const QRect &plot) const {
    /// @brief Grid lines color from QSS.
    painter.setPen(QPen(palette().color(QPalette::Active, QPalette::ToolTipBase), 1, Qt::DotLine));

    const int dbStep = 20;
    const int numSteps = static_cast<int>(std::abs(m_minDb) / dbStep);
    
    for (int i = 0; i <= numSteps; ++i) {
        const float db = -static_cast<float>(i * dbStep);
        if (db < m_minDb) break;
        
        const float t = (db - m_minDb) / (m_maxDb - m_minDb);
        const int y = plot.bottom() - static_cast<int>(t * plot.height());
        painter.drawLine(plot.left(), y, plot.right(), y);
    }
    
    if (std::abs(std::fmod(m_minDb, static_cast<float>(dbStep))) > 0.5f) {
        painter.drawLine(plot.left(), plot.bottom(), plot.right(), plot.bottom());
    }

    const float freqs[] = {100.0f, 1000.0f, 10000.0f};
    for (float f : freqs) {
        float t = (std::log10(f) - LOG_MIN) / (LOG_MAX - LOG_MIN);
        int x = plot.left() + static_cast<int>(t * plot.width());
        if (x > plot.left() && x < plot.right()) {
            painter.drawLine(x, plot.top(), x, plot.bottom());
        }
    }
}

void SpectrumView::drawCurve(QPainter &painter, const QRect &plot) const {
    if (m_magnitudeDb.empty() || plot.width() <= 0 || plot.height() <= 0) return;

    const int binCount = static_cast<int>(m_magnitudeDb.size());
    const float binHz = (m_sampleRate * 0.5f) / static_cast<float>(binCount);
    const float dbSpan = m_maxDb - m_minDb;

    /// @brief Curve color from QSS.
    painter.setPen(QPen(palette().highlight(), CURVE_WIDTH));
    
    QPainterPath path;
    bool first = true;

    for (int x = 0; x < plot.width(); ++x) {
        float t = static_cast<float>(x) / plot.width();
        float freq = std::pow(10.0f, LOG_MIN + t * (LOG_MAX - LOG_MIN));
        
        float binIdx = freq / binHz;
        int i0 = std::clamp(static_cast<int>(binIdx), 0, binCount - 1);
        
        float db = std::clamp(m_magnitudeDb[static_cast<std::size_t>(i0)], m_minDb, m_maxDb);
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
    /// @brief Label color from QSS.
    painter.setPen(palette().text().color());
    QFont font = painter.font();
    font.setPointSize(AXIS_LABEL_FONT_SIZE);
    painter.setFont(font);

    auto drawLabel = [&](float db) {
        const float t = (db - m_minDb) / (m_maxDb - m_minDb);
        const int y = plot.bottom() - static_cast<int>(t * plot.height());
        painter.drawText(QRect(0, y - 8, plot.left() - 4, 16),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(static_cast<int>(db)));
    };

    const int dbStepLabel = 20;
    const int numLabelSteps = static_cast<int>(std::abs(m_minDb) / dbStepLabel);

    for (int i = 0; i <= numLabelSteps; ++i) {
        const float db = -static_cast<float>(i * dbStepLabel);
        if (db < m_minDb) break;
        
        if (std::abs(db - m_minDb) > 8.0f) {
            drawLabel(db);
        }
    }
    drawLabel(m_minDb);

    const struct { float f; const char* lbl; } labels[] = {
        {20.0f, "20"}, {100.0f, "100"}, {1000.0f, "1k"}, {10000.0f, "10k"}, {20000.0f, "20k"}
    };
    
    for (auto const& l : labels) {
        float t = (std::log10(l.f) - LOG_MIN) / (LOG_MAX - LOG_MIN);
        int x = plot.left() + static_cast<int>(t * plot.width());
        painter.drawText(QRect(x - 20, plot.bottom() + 4, 40, 16),
                         Qt::AlignHCenter | Qt::AlignTop,
                         l.lbl);
    }
}

} // namespace Host::UI
