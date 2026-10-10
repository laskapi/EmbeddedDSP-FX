#ifndef EMBEDDEDDSP_HOST_SPECTRUMPANEL_H
#define EMBEDDEDDSP_HOST_SPECTRUMPANEL_H

#include <QFrame>

class QSlider;
class QLabel;

namespace Host::Core { class AppController; }

namespace Host::UI::Spectrum {

class SpectrumPlotHost;
class SpectrumAxisX;
class SpectrumAxisY;

/// @brief Container combining spectrum plot, axes, and zoom controls.
class SpectrumPanel : public QFrame {
    Q_OBJECT
public:
    explicit SpectrumPanel(Core::AppController& controller, QWidget *parent = nullptr);
    ~SpectrumPanel() override = default;

private:
    SpectrumPlotHost* m_spectrumPlotHost{nullptr};
    SpectrumAxisX* m_axisX{nullptr};
    SpectrumAxisY* m_axisY{nullptr};

    QSlider* m_dbSlider{nullptr};
    QLabel* m_zoomIcon{nullptr};
    QLabel* m_dbUnitLabel{nullptr};
};

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUMPANEL_H
