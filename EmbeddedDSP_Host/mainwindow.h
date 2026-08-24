#ifndef EMBEDDEDDSP_HOST_MAINWINDOW_H
#define EMBEDDEDDSP_HOST_MAINWINDOW_H

#include <EffectsRack.h>
#include <QMainWindow>
#include <vector>

#include "SerialManager.h"
#include "FftProcessor.h"
#include "SpectrumWidget.h"
#include "ConnectionToolbar.h"
#include "AudioFrameSimulator.h"
#include "Protocol/AudioFramePacket.h"

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
    void onAudioFrameReceived(const AudioFramePacket &frame);
    void onPortStatusChanged(bool isOpen, const QString &portName);
    void onSerialError(const QString &errorMessage);

private:
    void setupUiLayout();
    void wireConnectionToolbar();
    void wireAudioPipeline();
    void wireControlPipeline();

    Ui::MainWindow *ui{nullptr};
    SerialManager m_serialManager;
    FftProcessor m_fftProcessor{AUDIO_PACKET_SAMPLES};
    AudioFrameSimulator m_simulator;

    ConnectionToolbar *m_connectionToolbar{nullptr};
    SpectrumWidget *m_spectrumWidget{nullptr};
    EffectsRack *m_effectsRack{nullptr};

    std::vector<float> m_lastSpectrumDb;
};

#endif // EMBEDDEDDSP_HOST_MAINWINDOW_H