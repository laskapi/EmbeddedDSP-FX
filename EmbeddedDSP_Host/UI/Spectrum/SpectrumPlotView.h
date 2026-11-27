#ifndef EMBEDDEDDSP_HOST_SPECTRUMPLOTVIEW_H
#define EMBEDDEDDSP_HOST_SPECTRUMPLOTVIEW_H

#include "Models/EqCurveState.h"
#include "Protocol/ProtocolCommon.h"

#include <QVector>
#include <QWidget>
#include <vector>

class QPaintEvent;
class QRect;
class QPainter;

namespace Host::Core { class AppController; }

namespace Host::UI::Spectrum {

/// @brief FFT magnitude spectrum plot with optional EQ |H(f)| overlay.
class SpectrumPlotView : public QWidget {
    Q_OBJECT
public:
    explicit SpectrumPlotView(Core::AppController& controller, QWidget *parent = nullptr);
    ~SpectrumPlotView() override = default;

    void setDbRange(float minDb, float maxDb);
    void setEqCurves(const QVector<Host::Models::EqCurveState>& curves);

public slots:
    void updateSpectrum(const std::vector<float> &magnitudeDb);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    [[nodiscard]] QRect drawRect() const;
    void drawGrid(QPainter &painter, const QRect &plot) const;
    void drawCurve(QPainter &painter, const QRect &plot) const;
    void drawEqCurves(QPainter &painter, const QRect &plot) const;
    void drawEqZeroLine(QPainter &painter, const QRect &plot) const;

    Core::AppController& m_appController;
    std::vector<float> m_magnitudeDb;
    QVector<Host::Models::EqCurveState> m_eqCurves;

    float m_minDb{-100.0f};
    float m_maxDb{0.0f};
    float m_sampleRate{Protocol::SAMPLE_RATE_HZ};
};

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUMPLOTVIEW_H
