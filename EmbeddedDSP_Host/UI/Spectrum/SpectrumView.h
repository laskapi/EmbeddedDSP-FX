#ifndef EMBEDDEDDSP_HOST_SPECTRUMVIEW_H
#define EMBEDDEDDSP_HOST_SPECTRUMVIEW_H

#include <QWidget>
#include <vector>
#include <memory>

class QPaintEvent;
class QRect;
class QPainter;

namespace Host { class AppController; }

namespace Host::UI::Spectrum {

/// @brief Pure visualization component for real-time FFT magnitude spectrum.
class SpectrumView : public QWidget {
    Q_OBJECT
public:
    explicit SpectrumView(std::shared_ptr<AppController> controller, QWidget *parent = nullptr);
    ~SpectrumView() override = default;

    void setDbRange(float minDb, float maxDb);

public slots:
    void updateSpectrum(const std::vector<float> &magnitudeDb);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    [[nodiscard]] QRect drawRect() const;
    void drawGrid(QPainter &painter, const QRect &plot) const;
    void drawCurve(QPainter &painter, const QRect &plot) const;

    std::shared_ptr<AppController> m_appController;
    std::vector<float> m_magnitudeDb;
    
    float m_minDb{-100.0f};
    float m_maxDb{0.0f};
    float m_sampleRate{48000.0f};
};

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUMVIEW_H
