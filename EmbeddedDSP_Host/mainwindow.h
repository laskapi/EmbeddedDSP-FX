#ifndef EMBEDDEDDSP_HOST_MAINWINDOW_H
#define EMBEDDEDDSP_HOST_MAINWINDOW_H

#include <QMainWindow>
#include <memory>

namespace Host { class AppController; }
namespace Host::UI {
    class EffectRack;
    namespace Spectrum { class SpectrumPanel; }
    class ConnectionToolbar;
}

/// @brief Main application window managing layout and core UI components.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupUiLayout();
    void animateRack(bool show);
    
    // State and Logic
    std::shared_ptr<Host::AppController> m_appController;

    // UI Components
    Host::UI::ConnectionToolbar *m_connectionToolbar{nullptr};
    Host::UI::Spectrum::SpectrumPanel *m_spectrumPanel{nullptr};
    Host::UI::EffectRack *m_effectsRack{nullptr};
};

#endif // EMBEDDEDDSP_HOST_MAINWINDOW_H
