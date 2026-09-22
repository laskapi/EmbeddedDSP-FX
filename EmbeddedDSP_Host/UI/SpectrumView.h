#ifndef EMBEDDEDDSP_HOST_SPECTRUMVIEW_H
#define EMBEDDEDDSP_HOST_SPECTRUMVIEW_H

#include <QWidget>
#include <QColor>
#include <vector>

namespace Host {
    class StateManager;
    namespace UI {

/// @brief Visualization component for real-time FFT magnitude spectrum.
class SpectrumView : public QWidget {
    Q_OBJECT

public:
    explicit SpectrumView(StateManager* manager, QWidget *parent = nullptr);

    /// @brief Sets vertical display range.
    void setDbRange(float minDb, float maxDb);
    
    /// @brief Sets sample rate for frequency axis labels.
    void setSampleRate(float sampleRateHz);
    
    /// @brief Sets FFT size for smoothing calculations.
    void setFftSize(int fftSize);

public slots:
    /// @brief Updates internal data and redraws the spectrum.
    void updateSpectrum(const std::vector<float> &magnitudeDb);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawBackground(QPainter &painter, const QRect &plot) const;
    void drawGrid(QPainter &painter, const QRect &plot) const;
    void drawBars(QPainter &painter, const QRect &plot) const;
    void drawAxesLabels(QPainter &painter, const QRect &plot) const;

    [[nodiscard]] QRect plotRect() const;

    std::vector<float> m_magnitudeDb;
    float m_minDb{-100.0f};
    float m_maxDb{0.0f};
    float m_sampleRate{48000.0f};
    int m_fftSize{128};

    QColor m_bgColor{24, 26, 30};
    QColor m_gridColor{55, 60, 70};
    QColor m_barColor{80, 200, 140};
    QColor m_textColor{180, 185, 195};
};

} // namespace UI
} // namespace Host

#endif // EMBEDDEDDSP_HOST_SPECTRUMVIEW_H
