#ifndef EMBEDDEDDSP_HOST_SPECTRUMPLOTVIEW_H
#define EMBEDDEDDSP_HOST_SPECTRUMPLOTVIEW_H

#include <QWidget>
#include <vector>

class QPaintEvent;
class QRect;
class QPainter;

namespace Host::Core { class AppController; }

namespace Host::UI::Spectrum {

/// @brief FFT magnitude spectrum plot (paint-only, no overlay).
class SpectrumPlotView : public QWidget {
    Q_OBJECT
public:
    explicit SpectrumPlotView(Core::AppController& controller, QWidget *parent = nullptr);
    ~SpectrumPlotView() override = default;

    void setDbRange(float minDb, float maxDb);

public slots:
    void updateSpectrum(const std::vector<float> &magnitudeDb);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    [[nodiscard]] QRect drawRect() const;
    void drawGrid(QPainter &painter, const QRect &plot) const;
    void drawCurve(QPainter &painter, const QRect &plot) const;

    Core::AppController& m_appController;
    std::vector<float> m_magnitudeDb;

    float m_minDb{-100.0f};
    float m_maxDb{0.0f};
    float m_sampleRate{48000.0f};
};

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUMPLOTVIEW_H
