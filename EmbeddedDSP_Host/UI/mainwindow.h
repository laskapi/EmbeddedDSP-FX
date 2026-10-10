#ifndef EMBEDDEDDSP_HOST_MAINWINDOW_H
#define EMBEDDEDDSP_HOST_MAINWINDOW_H

#include <QMainWindow>

namespace Host::Core { class AppController; }
namespace Host::UI {
    namespace Effects { class EffectRack; }
    namespace Spectrum { class SpectrumPanel; }
    class ConnectionToolbar;
}

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
    void animateRack(bool show);
    
    // State and Logic
    Core::AppController& m_appController;

    // UI Components
    ConnectionToolbar *m_connectionToolbar{nullptr};
    Spectrum::SpectrumPanel *m_spectrumPanel{nullptr};
    Effects::EffectRack *m_effectsRack{nullptr};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_MAINWINDOW_H
