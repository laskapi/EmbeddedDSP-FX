#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QVBoxLayout>
#include <QSerialPortInfo>
#include <QDebug>
#include <span>
#include <algorithm>

// Toggle host-side audio simulation (no MCU required).
// Set to 0 to hide Demo UI.
#define EMBEDDED_DSP_HOST_ENABLE_SIMULATOR 1

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("EmbeddedDSP Host"));

    auto *toolbar = new QWidget(this);
    m_portCombo = new QComboBox(toolbar);
    m_connectButton = new QPushButton(QStringLiteral("Connect"), toolbar);

    m_topRow = new QHBoxLayout(toolbar);
    m_topRow->setContentsMargins(8, 8, 8, 8);
    m_topRow->addWidget(m_portCombo);
    m_topRow->addWidget(m_connectButton);
    m_topRow->addStretch();

    m_spectrumWidget = new SpectrumWidget(this);
    m_spectrumWidget->setSampleRate(48000.0f);
    m_spectrumWidget->setFftSize(AUDIO_PACKET_SAMPLES);
    m_spectrumWidget->setDbRange(-100.0f, 0.0f);

    auto *layout = new QVBoxLayout();
    layout->addWidget(toolbar);
    layout->addWidget(m_spectrumWidget, 1);
    ui->centralwidget->setLayout(layout);
    ui->centralwidget->setLayout(layout);

    refreshPortList();

    connect(m_connectButton, &QPushButton::clicked,
            this, &MainWindow::onConnectClicked);
    connect(&m_serialManager, &SerialManager::audioFrameReceived,
            this, &MainWindow::onAudioFrameReceived);
    connect(&m_serialManager, &SerialManager::portStatusChanged,
            this, &MainWindow::onPortStatusChanged);
    connect(&m_serialManager, &SerialManager::errorOccurred,
            this, &MainWindow::onSerialError);

#if EMBEDDED_DSP_HOST_ENABLE_SIMULATOR
    setupAudioSimulator(); // one call: Demo button + signal wiring
#endif

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshPortList()
{
    m_portCombo->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        const QString label = QStringLiteral("%1  (%2)")
        .arg(info.portName(), info.description());
        m_portCombo->addItem(label, info.portName());
    }

    m_connectButton->setEnabled(m_portCombo->count() > 0);
}

void MainWindow::onConnectClicked()
{
    if (m_simulator.isRunning()) {
        m_simulator.stop();
    }

    if (m_serialManager.isOpen()) {
        m_serialManager.closePort();
        return;
    }

    const QString portName = m_portCombo->currentData().toString();
    if (portName.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("No serial port selected."));
        return;
    }

    m_serialManager.openPort(portName, 115200);
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
    m_connectButton->setText(isOpen ? QStringLiteral("Disconnect")
                                    : QStringLiteral("Connect"));
    m_portCombo->setEnabled(!isOpen);

    statusBar()->showMessage(isOpen
                                 ? QStringLiteral("Connected: %1").arg(portName)
                                 : QStringLiteral("Disconnected: %1").arg(portName));
}

void MainWindow::onSerialError(const QString &errorMessage)
{
    qDebug() << "Serial error:" << errorMessage;
    statusBar()->showMessage(errorMessage, 4000);
}

void MainWindow::setupAudioSimulator()
{
    m_demoButton = new QPushButton(QStringLiteral("Demo"), m_portCombo->parentWidget());
    m_topRow->addWidget(m_demoButton);

    connect(m_demoButton, &QPushButton::clicked, this, [this]() {
        if (m_simulator.isRunning()) {
            m_simulator.stop();
            return;
        }
        if (m_serialManager.isOpen()) {
            m_serialManager.closePort();
        }
        m_simulator.start();
    });

    connect(&m_simulator, &AudioFrameSimulator::audioFrameReceived,
            this, &MainWindow::onAudioFrameReceived);

    connect(&m_simulator, &AudioFrameSimulator::runningChanged,
            this, [this](bool running) {
                m_demoButton->setText(running ? QStringLiteral("Stop Demo")
                                              : QStringLiteral("Demo"));
                if (running) {
                    statusBar()->showMessage(
                        QStringLiteral("Demo: %1 Hz synthetic tone")
                            .arg(static_cast<double>(m_simulator.frequencyHz()), 0, 'f', 0));
                }
            });
}