#ifndef EMBEDDEDDSP_HOST_MAINWINDOW_H
#define EMBEDDEDDSP_HOST_MAINWINDOW_H

#include <QMainWindow>
#include <vector>

#include "SerialManager.h"
#include "FftProcessor.h"
#include "AudioFrameSimulator.h"
#include <Protocol/AudioFramePacket.h>

// Forward declarations in UI namespace
namespace UI {
    class ConnectionToolbar;
    class EffectsRack;
    class SpectrumView;
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAudioFrameReceived(const Protocol::AudioFramePacket &frame);
    void onPortStatusChanged(bool isOpen, const QString &portName);
    void onSerialError(const QString &errorMessage);

private:
    void setupUiLayout();
    void wireConnectionToolbar();
    void wireAudioPipeline();
    void wireControlPipeline();

    Ui::MainWindow *ui{nullptr};
    SerialManager m_serialManager;
    FftProcessor m_fftProcessor{Protocol::AUDIO_SAMPLES};
    AudioFrameSimulator m_simulator;

    UI::ConnectionToolbar *m_connectionToolbar{nullptr};
    UI::SpectrumView *m_spectrumView{nullptr};
    UI::EffectsRack *m_effectsRack{nullptr};

    std::vector<float> m_lastSpectrumDb;
};

#endif // EMBEDDEDDSP_HOST_MAINWINDOW_H
