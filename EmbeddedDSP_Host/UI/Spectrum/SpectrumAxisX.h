#ifndef EMBEDDEDDSP_HOST_SPECTRUMAXISX_H
#define EMBEDDEDDSP_HOST_SPECTRUMAXISX_H

#include <QWidget>

namespace Host::UI::Spectrum {

/// @brief Horizontal frequency axis labels for the spectrum view.
class SpectrumAxisX : public QWidget {
    Q_OBJECT
public:
    explicit SpectrumAxisX(QWidget* parent = nullptr);
    ~SpectrumAxisX() override = default;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void updateMetrics();
    int m_labelHeight{16};
    int m_maxLabelWidth{40};
};

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUMAXISX_H
