#ifndef EMBEDDEDDSP_HOST_MAINWINDOW_H
#define EMBEDDEDDSP_HOST_MAINWINDOW_H

#include <QMainWindow>
#include <vector>

#include "FftProcessor.h"
#include "AudioFrameSimulator.h"
#include <Protocol/AudioFramePacket.h>

namespace UI {
    class ConnectionToolbar;
    class EffectsRack;
    class SpectrumView;
    class StateManager;
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Main window of the EmbeddedDSP Host application.
 * 
 * Orchestrates the overall UI layout, initializes the StateManager,
 * and handles the audio visualization pipeline.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    /** @brief Handles audio frames for FFT processing and visualization. */
    void onAudioFrameReceived(const Protocol::AudioFramePacket &frame);

    /** @brief Responds to device connection status changes. */
    void onConnectionChanged(bool connected, const QString &portName);

    /** @brief Displays serial communication errors in the status bar. */
    void onSerialError(const QString &errorMessage);

private:
    /** @brief Sets up the main layout and widgets. */
    void setupUiLayout();

    /** @brief Connects the connection toolbar logic to the system. */
    void wireConnectionToolbar();

    /** @brief Connects audio frame streams to the FFT processor. */
    void wireAudioPipeline();

    /** @brief Connects UI actions to the StateManager logic. */
    void wireControlPipeline();

    Ui::MainWindow *ui{nullptr};
    
    // State and Logic
    UI::StateManager *m_stateManager{nullptr};
    FftProcessor m_fftProcessor{Protocol::AUDIO_SAMPLES};
    AudioFrameSimulator m_simulator;

    // UI Components
    UI::ConnectionToolbar *m_connectionToolbar{nullptr};
    UI::SpectrumView *m_spectrumView{nullptr};
    UI::EffectsRack *m_effectsRack{nullptr};

    std::vector<float> m_lastSpectrumDb;
};

#endif // EMBEDDEDDSP_HOST_MAINWINDOW_H
