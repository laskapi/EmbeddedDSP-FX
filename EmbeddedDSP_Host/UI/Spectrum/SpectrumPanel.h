#ifndef EMBEDDEDDSP_HOST_SPECTRUMPANEL_H
#define EMBEDDEDDSP_HOST_SPECTRUMPANEL_H

#include <QFrame>
#include <memory>

class QSlider;
class QLabel;
class QGridLayout;

namespace Host { class AppController; }

namespace Host::UI::Spectrum {

class SpectrumView;
class SpectrumAxisX;
class SpectrumAxisY;

/**
 * @brief Container that combines SpectrumView with modular axes and control widgets.
 */
class SpectrumPanel : public QFrame {
    Q_OBJECT
public:
    /// @brief Main spectrum panel constructor.
    explicit SpectrumPanel(std::shared_ptr<AppController> controller, QWidget *parent = nullptr);
    ~SpectrumPanel() override = default;

    void setWelcomeVisible(bool visible);

private:
    std::shared_ptr<AppController> m_appController;
    SpectrumView*  m_spectrumView{nullptr};
    SpectrumAxisX* m_axisX{nullptr};
    SpectrumAxisY* m_axisY{nullptr};
    
    QSlider* m_dbSlider{nullptr};
    QLabel* m_zoomIcon{nullptr};
    QLabel* m_dbUnitLabel{nullptr};
    QLabel* m_welcomeLabel{nullptr};
};

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUMPANEL_H
