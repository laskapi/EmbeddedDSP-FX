#ifndef EMBEDDEDDSP_HOST_MAINWINDOW_H
#define EMBEDDEDDSP_HOST_MAINWINDOW_H

#include <QMainWindow>
#include <memory>

namespace Host { class AppController; }
namespace Host::UI {
    class ConnectionToolbar;
    class EffectRack;
    namespace Spectrum { class SpectrumPanel; }
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/** @brief Main window of the EmbeddedDSP Host application. */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    /** @brief Sets up the main layout and widgets. */
    void setupUiLayout();
    void animateRack(bool show);

    Ui::MainWindow *ui{nullptr};
    
    // State and Logic
    std::shared_ptr<Host::AppController> m_appController;

    // UI Components
    Host::UI::ConnectionToolbar *m_connectionToolbar{nullptr};
    Host::UI::Spectrum::SpectrumPanel *m_spectrumPanel{nullptr};
    Host::UI::EffectRack *m_effectsRack{nullptr};
};

#endif // EMBEDDEDDSP_HOST_MAINWINDOW_H
