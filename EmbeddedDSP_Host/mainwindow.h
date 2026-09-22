#ifndef EMBEDDEDDSP_HOST_MAINWINDOW_H
#define EMBEDDEDDSP_HOST_MAINWINDOW_H

#include <QMainWindow>

namespace Host {
    class StateManager;
    namespace UI {
        class ConnectionToolbar;
        class EffectsRack;
        class SpectrumView;
    }
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

    Ui::MainWindow *ui{nullptr};
    
    // State and Logic
    Host::StateManager *m_stateManager{nullptr};

    // UI Components
    Host::UI::ConnectionToolbar *m_connectionToolbar{nullptr};
    Host::UI::SpectrumView *m_spectrumView{nullptr};
    Host::UI::EffectsRack *m_effectsRack{nullptr};
};

#endif // EMBEDDEDDSP_HOST_MAINWINDOW_H
