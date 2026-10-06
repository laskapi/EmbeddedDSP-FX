#ifndef EMBEDDEDDSP_HOST_SPECTRUMAXISY_H
#define EMBEDDEDDSP_HOST_SPECTRUMAXISY_H

#include <QWidget>

namespace Host::UI::Spectrum {

/// @brief Vertical dB axis labels for the spectrum view.
class SpectrumAxisY : public QWidget {
    Q_OBJECT
public:
    explicit SpectrumAxisY(QWidget* parent = nullptr);
    ~SpectrumAxisY() override = default;

    void setDbRange(float minDb, float maxDb);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void updateMetrics();
    int m_labelHeight{16};
    int m_maxLabelWidth{32};

    float m_minDb{-100.0f};
    float m_maxDb{0.0f};
};

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUMAXISY_H
