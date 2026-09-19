#ifndef EMBEDDEDDSP_HOST_SPECTRUMVIEW_H
#define EMBEDDEDDSP_HOST_SPECTRUMVIEW_H

#include <QWidget>
#include <QColor>
#include <vector>

namespace UI {

/**
 * @brief Real-time magnitude spectrum visualization.
 */
class SpectrumView : public QWidget {
    Q_OBJECT

public:
    explicit SpectrumView(QWidget *parent = nullptr);

    /** @brief Sets the vertical dB display range. */
    void setDbRange(float minDb, float maxDb);
    
    /** @brief Configures frequency axis labels. */
    void setSampleRate(float sampleRateHz);
    
    /** @brief Sets FFT size for smoothing logic. */
    void setFftSize(int fftSize);

public slots:
    /** @brief Updates the internal data and triggers a repaint. */
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

#endif // EMBEDDEDDSP_HOST_SPECTRUMVIEW_H
