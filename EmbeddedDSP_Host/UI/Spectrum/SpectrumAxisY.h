#ifndef EMBEDDEDDSP_HOST_SPECTRUMAXISY_H
#define EMBEDDEDDSP_HOST_SPECTRUMAXISY_H

#include <QWidget>

class QPainter;

namespace Host::UI::Spectrum {

/// @brief Vertical dB axis labels for the spectrum view.
class SpectrumAxisY : public QWidget {
    Q_OBJECT
public:
    explicit SpectrumAxisY(QWidget* parent = nullptr);
    ~SpectrumAxisY() override = default;

    void setFftDbRange(float minDb, float maxDb);
    void setEqMode(bool eqMode);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void updateMetrics();
    void drawFftTicks(QPainter& painter, int marginV, int usableHeight) const;
    void drawEqTicks(QPainter& painter, int marginV, int usableHeight) const;

    int m_labelHeight{16};
    int m_maxLabelWidth{32};

    float m_fftMinDb{-100.0f};
    float m_fftMaxDb{0.0f};
    bool m_eqMode{false};
};

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUMAXISY_H
