#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QVBoxLayout>
#include <QDebug>
#include <span>
#include <algorithm>

#include "EffectsRack.h"

// Toggle host-side audio simulation (no MCU required).
// Set to 0 to hide Demo button in ConnectionToolbar.
#define EMBEDDED_DSP_HOST_ENABLE_SIMULATOR 1

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("EmbeddedDSP Host"));

    setupUiLayout();
    wireConnectionToolbar();
    wireAudioPipeline();
    wireControlPipeline(); // Don't forget to wire control signals!
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUiLayout()
{
    m_connectionToolbar = new ConnectionToolbar(
        EMBEDDED_DSP_HOST_ENABLE_SIMULATOR != 0, this);

    m_spectrumWidget = new SpectrumWidget(this);
    m_spectrumWidget->setSampleRate(48000.0f);
    m_spectrumWidget->setFftSize(AUDIO_PACKET_SAMPLES);
    m_spectrumWidget->setDbRange(-100.0f, 0.0f);

    // Initialize the new effects rack
    m_effectsRack = new EffectsRack(this);

    auto *layout = new QVBoxLayout();
    layout->addWidget(m_connectionToolbar);
    layout->addWidget(m_spectrumWidget, 1);
    layout->addWidget(m_effectsRack, 1); // Add rack below spectrum

    ui->centralwidget->setLayout(layout);
}

void MainWindow::wireControlPipeline()
{
    // Using lambda to handle the bool return from sendControlPacket
    // and provide feedback to the user via status bar.
    connect(m_effectsRack, &EffectsRack::controlPacketReady, this, [this](const ControlPacket::ControlPacket &pkt) {
        if (!m_serialManager.sendControlPacket(pkt)) {
            statusBar()->showMessage(tr("Serial Error: Command not sent"), 2000);
        }
    });
}

void MainWindow::wireConnectionToolbar()
{
    connect(m_connectionToolbar, &ConnectionToolbar::connectRequested,
            this, [this](const QString &portName) {
                if (m_simulator.isRunning()) {
                    m_simulator.stop();
                }

                if (portName.isEmpty()) {
                    statusBar()->showMessage(QStringLiteral("No serial port selected."));
                    return;
                }

                m_serialManager.openPort(portName, 115200);
            });

    connect(m_connectionToolbar, &ConnectionToolbar::disconnectRequested,
            this, [this]() {
                m_serialManager.closePort();
            });

#if EMBEDDED_DSP_HOST_ENABLE_SIMULATOR
    connect(m_connectionToolbar, &ConnectionToolbar::demoStartRequested,
            this, [this]() {
                if (m_serialManager.isOpen()) {
                    m_serialManager.closePort();
                }
                m_simulator.start();
            });

    connect(m_connectionToolbar, &ConnectionToolbar::demoStopRequested,
            this, [this]() {
                m_simulator.stop();
            });

    connect(&m_simulator, &AudioFrameSimulator::runningChanged,
            this, [this](bool running) {
                m_connectionToolbar->setDemoRunning(running);
                if (running) {
                    statusBar()->showMessage(
                        QStringLiteral("Demo: %1 Hz synthetic tone")
                            .arg(static_cast<double>(m_simulator.frequencyHz()), 0, 'f', 0));
                }
            });
#endif
}

void MainWindow::wireAudioPipeline()
{
    connect(&m_serialManager, &SerialManager::audioFrameReceived,
            this, &MainWindow::onAudioFrameReceived);

#if EMBEDDED_DSP_HOST_ENABLE_SIMULATOR
    connect(&m_simulator, &AudioFrameSimulator::audioFrameReceived,
            this, &MainWindow::onAudioFrameReceived);
#endif

    connect(&m_serialManager, &SerialManager::portStatusChanged,
            this, &MainWindow::onPortStatusChanged);
    connect(&m_serialManager, &SerialManager::errorOccurred,
            this, &MainWindow::onSerialError);
}

void MainWindow::onAudioFrameReceived(const AudioFramePacket &frame)
{
    const std::span<const int16_t> pcm{frame.samples};
    m_lastSpectrumDb = m_fftProcessor.processFrame(pcm);

    if (m_spectrumWidget) {
        m_spectrumWidget->updateSpectrum(m_lastSpectrumDb);
    }

    if (!m_lastSpectrumDb.empty()) {
        const auto peakIt = std::max_element(m_lastSpectrumDb.begin(),
                                             m_lastSpectrumDb.end());
        const int peakBin = static_cast<int>(
            std::distance(m_lastSpectrumDb.begin(), peakIt));
        statusBar()->showMessage(
            QStringLiteral("seq=%1  peakBin=%2  peak=%3 dB")
                .arg(frame.sequenceNumber)
                .arg(peakBin)
                .arg(static_cast<double>(*peakIt), 0, 'f', 1));
    }
}

void MainWindow::onPortStatusChanged(bool isOpen, const QString &portName)
{
    m_connectionToolbar->setConnected(isOpen);

    statusBar()->showMessage(isOpen
                                 ? QStringLiteral("Connected: %1").arg(portName)
                                 : QStringLiteral("Disconnected: %1").arg(portName));
}

void MainWindow::onSerialError(const QString &errorMessage)
{
    qDebug() << "Serial error:" << errorMessage;
    statusBar()->showMessage(errorMessage, 4000);
}