#ifndef EMBEDDEDDSP_HOST_SPECTRUMWIDGET_H
#define EMBEDDEDDSP_HOST_SPECTRUMWIDGET_H

#include <QWidget>
#include <QColor>
#include <vector>

/**
 * @brief Real-time magnitude spectrum view (dB bins from FftProcessor).
 */
class SpectrumWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpectrumWidget(QWidget *parent = nullptr);

    void setDbRange(float minDb, float maxDb);
    void setSampleRate(float sampleRateHz);
    void setFftSize(int fftSize);

public slots:
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

#endif // EMBEDDEDDSP_HOST_SPECTRUMWIDGET_H