#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QStatusBar>
#include <span>
#include <algorithm>

#define EMBEDDED_DSP_HOST_ENABLE_SIMULATOR 1

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("EmbeddedDSP Host - Universal Control"));

    setupUiLayout();
    wireConnectionToolbar();
    wireAudioPipeline();
    wireControlPipeline();

    QString autoPort = SerialManager::findDevicePort();
    if (!autoPort.isEmpty()) {
        statusBar()->showMessage(tr("Device detected on %1. Connecting...").arg(autoPort), 3000);
        
        if (m_serialManager.openPort(autoPort, 115200)) {
            Protocol::ControlPacket syncPkt;
            syncPkt.command = Protocol::Command::GetState;
            syncPkt.applyCRC();
            m_serialManager.sendControlPacket(syncPkt);
        }
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUiLayout() {
    m_connectionToolbar = new ConnectionToolbar(EMBEDDED_DSP_HOST_ENABLE_SIMULATOR != 0, this);
    m_spectrumWidget = new SpectrumWidget(this);
    m_effectsRack = new EffectsRack(this);

    auto *layout = new QVBoxLayout();
    layout->addWidget(m_connectionToolbar);
    layout->addWidget(m_spectrumWidget, 1);
    layout->addWidget(m_effectsRack, 1);

    ui->centralwidget->setLayout(layout);
}

void MainWindow::wireControlPipeline() {
    connect(m_effectsRack, &EffectsRack::controlPacketReady, this, [this](const Protocol::ControlPacket &pkt) {
        m_serialManager.sendControlPacket(pkt);
    });

    connect(&m_serialManager, &SerialManager::controlPacketReceived, this, [this](const Protocol::ControlPacket &pkt) {
        m_effectsRack->syncFromDevice(pkt);
    });

    connect(&m_serialManager, &SerialManager::manifestReceived, 
            m_effectsRack, &EffectsRack::onManifestReceived);
}

void MainWindow::wireConnectionToolbar() {
    connect(m_connectionToolbar, &ConnectionToolbar::connectRequested, this, [this](const QString &portName) {
        if (m_simulator.isRunning()) m_simulator.stop();

        if (m_serialManager.openPort(portName, 115200)) {
            Protocol::ControlPacket syncPkt;
            syncPkt.command = Protocol::Command::GetState;
            syncPkt.applyCRC();
            m_serialManager.sendControlPacket(syncPkt);
        }
    });

    connect(m_connectionToolbar, &ConnectionToolbar::disconnectRequested, this, [this]() {
        m_serialManager.closePort();
    });

#if EMBEDDED_DSP_HOST_ENABLE_SIMULATOR
    connect(m_connectionToolbar, &ConnectionToolbar::demoStartRequested, this, [this]() {
        if (m_serialManager.isOpen()) m_serialManager.closePort();
        m_simulator.start();
    });

    connect(m_connectionToolbar, &ConnectionToolbar::demoStopRequested, this, [this]() {
        m_simulator.stop();
    });

    connect(&m_simulator, &AudioFrameSimulator::runningChanged, this, [this](bool running) {
        m_connectionToolbar->setDemoRunning(running);
        if (running) statusBar()->showMessage(tr("Simulator Running"), 2000);
    });
#endif
}

void MainWindow::wireAudioPipeline() {
    connect(&m_serialManager, &SerialManager::audioFrameReceived, this, &MainWindow::onAudioFrameReceived);

#if EMBEDDED_DSP_HOST_ENABLE_SIMULATOR
    connect(&m_simulator, &AudioFrameSimulator::audioFrameReceived, this, &MainWindow::onAudioFrameReceived);
#endif

    connect(&m_serialManager, &SerialManager::portStatusChanged, this, &MainWindow::onPortStatusChanged);
    connect(&m_serialManager, &SerialManager::errorOccurred, this, &MainWindow::onSerialError);
}

void MainWindow::onAudioFrameReceived(const Protocol::AudioFramePacket &frame) {
    const std::span<const int16_t> pcm{frame.samples};
    auto spectrum = m_fftProcessor.processFrame(pcm);
    if (m_spectrumWidget) m_spectrumWidget->updateSpectrum(spectrum);
}

void MainWindow::onPortStatusChanged(bool isOpen, const QString &portName) {
    m_connectionToolbar->setConnected(isOpen);
    statusBar()->showMessage(isOpen ? tr("Connected to %1").arg(portName) : tr("Disconnected"));
    if(!isOpen){
        m_effectsRack->clear();
    }
}

void MainWindow::onSerialError(const QString &errorMessage) {
    statusBar()->showMessage(tr("Error: %1").arg(errorMessage), 5000);
}
