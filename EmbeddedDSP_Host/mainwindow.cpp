#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "UI/ConnectionToolbar.h"
#include "UI/EffectsRack.h"
#include "UI/SpectrumView.h"
#include "UI/StateManager.h"
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

    m_stateManager = new UI::StateManager(this);

    setupUiLayout();
    wireConnectionToolbar();
    wireAudioPipeline();
    wireControlPipeline();

    QString autoPort = SerialManager::findDevicePort();
    if (!autoPort.isEmpty()) {
        statusBar()->showMessage(tr("Device detected on %1. Connecting...").arg(autoPort), 3000);
        
        if (m_stateManager->serial().openPort(autoPort, 115200)) {
            m_stateManager->requestSync();
        }
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUiLayout() {
    m_connectionToolbar = new UI::ConnectionToolbar(EMBEDDED_DSP_HOST_ENABLE_SIMULATOR != 0, this);
    m_spectrumView = new UI::SpectrumView(this);
    m_effectsRack = new UI::EffectsRack(m_stateManager, this);

    auto *layout = new QVBoxLayout();
    layout->addWidget(m_connectionToolbar);
    layout->addWidget(m_spectrumView, 1);
    layout->addWidget(m_effectsRack, 1);

    ui->centralwidget->setLayout(layout);
}

void MainWindow::wireControlPipeline() {
    connect(m_stateManager, &UI::StateManager::deviceStateUpdated, 
            m_effectsRack, &UI::EffectsRack::syncFromDevice);

    connect(m_stateManager, &UI::StateManager::manifestProcessed, 
            m_effectsRack, &UI::EffectsRack::onManifestProcessed);

    connect(m_stateManager, &UI::StateManager::connectionChanged, 
            this, &MainWindow::onConnectionChanged);
}

void MainWindow::wireConnectionToolbar() {
    connect(m_connectionToolbar, &UI::ConnectionToolbar::connectRequested, this, [this](const QString &portName) {
        if (m_simulator.isRunning()) m_simulator.stop();

        if (m_stateManager->serial().openPort(portName, 115200)) {
            m_stateManager->requestSync();
        }
    });

    connect(m_connectionToolbar, &UI::ConnectionToolbar::disconnectRequested, this, [this]() {
        m_stateManager->serial().closePort();
    });

#if EMBEDDED_DSP_HOST_ENABLE_SIMULATOR
    connect(m_connectionToolbar, &UI::ConnectionToolbar::demoStartRequested, this, [this]() {
        if (m_stateManager->serial().isOpen()) m_stateManager->serial().closePort();
        m_simulator.start();
    });

    connect(m_connectionToolbar, &UI::ConnectionToolbar::demoStopRequested, this, [this]() {
        m_simulator.stop();
    });

    connect(&m_simulator, &AudioFrameSimulator::runningChanged, this, [this](bool running) {
        m_connectionToolbar->setDemoRunning(running);
        if (running) statusBar()->showMessage(tr("Simulator Running"), 2000);
    });
#endif
}

void MainWindow::wireAudioPipeline() {
    connect(&m_stateManager->serial(), &SerialManager::audioFrameReceived, this, &MainWindow::onAudioFrameReceived);

#if EMBEDDED_DSP_HOST_ENABLE_SIMULATOR
    connect(&m_simulator, &AudioFrameSimulator::audioFrameReceived, this, &MainWindow::onAudioFrameReceived);
#endif

    connect(&m_stateManager->serial(), &SerialManager::errorOccurred, this, &MainWindow::onSerialError);
}

void MainWindow::onAudioFrameReceived(const Protocol::AudioFramePacket &frame) {
    const std::span<const int16_t> pcm{frame.samples};
    auto spectrum = m_fftProcessor.processFrame(pcm);
    if (m_spectrumView) m_spectrumView->updateSpectrum(spectrum);
}

void MainWindow::onConnectionChanged(bool isOpen, const QString &portName) {
    m_connectionToolbar->setConnected(isOpen);
    statusBar()->showMessage(isOpen ? tr("Connected to %1").arg(portName) : tr("Disconnected"));
    if(!isOpen){
        m_effectsRack->clear();
    }
}

void MainWindow::onSerialError(const QString &errorMessage) {
    statusBar()->showMessage(tr("Error: %1").arg(errorMessage), 5000);
}
