#ifndef EMBEDDEDDSP_HOST_SPECTRUMVIEW_H
#define EMBEDDEDDSP_HOST_SPECTRUMVIEW_H

#include <QWidget>
#include <vector>
#include <memory>

class QPaintEvent;
class QRect;
class QPainter;

namespace Host { class AppController; }

namespace Host::UI {

/**
 * @brief Pure visualization component for real-time FFT magnitude spectrum.
 */
class SpectrumView : public QWidget {
    Q_OBJECT

public:
    static constexpr int AXIS_BOTTOM_MARGIN = 24;

    explicit SpectrumView(std::shared_ptr<AppController> controller, QWidget *parent = nullptr);
    ~SpectrumView() override = default;

    /// @brief Sets vertical display range.
    void setDbRange(float minDb, float maxDb);

public slots:
    void updateSpectrum(const std::vector<float> &magnitudeDb);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawBackground(QPainter &painter, const QRect &plot) const;
    void drawGrid(QPainter &painter, const QRect &plot) const;
    void drawCurve(QPainter &painter, const QRect &plot) const;
    void drawAxesLabels(QPainter &painter, const QRect &plot) const;

    [[nodiscard]] QRect plotRect() const;

    std::shared_ptr<AppController> m_appController;
    std::vector<float> m_magnitudeDb;
    
    float m_minDb{-100.0f};
    float m_maxDb{0.0f};
    float m_sampleRate{48000.0f};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_SPECTRUMVIEW_H
