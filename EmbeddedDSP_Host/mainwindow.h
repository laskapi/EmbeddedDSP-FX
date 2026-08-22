#ifndef EMBEDDEDDSP_HOST_MAINWINDOW_H
#define EMBEDDEDDSP_HOST_MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <vector>

#include "SerialManager.h"
#include "FftProcessor.h"
#include "SpectrumWidget.h"
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
    void onConnectClicked();
    void onAudioFrameReceived(const AudioFramePacket &frame);
    void onPortStatusChanged(bool isOpen, const QString &portName);
    void onSerialError(const QString &errorMessage);

private:
    void refreshPortList();
    void setupAudioSimulator();

    Ui::MainWindow *ui{nullptr};
    SerialManager m_serialManager;
    FftProcessor m_fftProcessor{AUDIO_PACKET_SAMPLES};
    AudioFrameSimulator m_simulator;

    QHBoxLayout *m_topRow{nullptr};
    QComboBox *m_portCombo{nullptr};
    QPushButton *m_connectButton{nullptr};
    QPushButton *m_demoButton{nullptr};
    SpectrumWidget *m_spectrumWidget{nullptr};
    std::vector<float> m_lastSpectrumDb;
};

#endif // EMBEDDEDDSP_HOST_MAINWINDOW_H