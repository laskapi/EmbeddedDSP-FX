#ifndef EMBEDDEDDSP_HOST_MAINWINDOW_H
#define EMBEDDEDDSP_HOST_MAINWINDOW_H

#include <QMainWindow>

namespace Host::Core { class AppController; }
namespace Host::UI {
    namespace Effects {
    class EffectRack;
    class MasterVolumeControl;
    class PresetBar;
    }
    namespace Spectrum { class SpectrumPanel; }
    class ConnectionToolbar;
}

class QWidget;

namespace Host::UI {

/// @brief Main application window managing layout and core UI components.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(Core::AppController& controller, QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupUiLayout();
    void animateFxStrip(bool show);

    /// @brief Probe one hidden EffectItem locked to kMaxEffectParams; cache strip height.
    [[nodiscard]] int measureFxStripHeight();

    /// @brief Raise min height / grow window if needed (never shrinks).
    void ensureWindowFitsFxStrip(int fxStripHeight);

    Core::AppController& m_appController;

    ConnectionToolbar *m_connectionToolbar{nullptr};
    Effects::PresetBar *m_presetBar{nullptr};
    Spectrum::SpectrumPanel *m_spectrumPanel{nullptr};
    QWidget *m_fxStrip{nullptr};
    Effects::EffectRack *m_effectsRack{nullptr};
    Effects::MasterVolumeControl *m_masterVolume{nullptr};
    int m_fxStripHeight{0};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_MAINWINDOW_H
