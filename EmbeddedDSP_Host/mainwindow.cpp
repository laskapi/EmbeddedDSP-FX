#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UI/ConnectionToolbar.h"
#include "UI/EffectsRack.h"
#include "UI/SpectrumView.h"
#include "StateManager.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QStatusBar>

#define EMBEDDED_DSP_HOST_ENABLE_SIMULATOR 1

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("EmbeddedDSP Host - Universal Control"));

    m_stateManager = new Host::StateManager(this);

    setupUiLayout();

    // Device events wiring
    connect(m_stateManager, &Host::StateManager::connectionChanged, this, [this](bool connected, const QString &portName) {
        statusBar()->showMessage(connected ? tr("Connected to %1").arg(portName) : tr("Disconnected"));
    });

    connect(m_stateManager, &Host::StateManager::errorOccurred, this, [this](const QString &errorMessage) {
        statusBar()->showMessage(tr("Error: %1").arg(errorMessage), 5000);
    });

    // Initial auto-connect
    m_stateManager->connect();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUiLayout() {
    m_connectionToolbar = new Host::UI::ConnectionToolbar(m_stateManager, EMBEDDED_DSP_HOST_ENABLE_SIMULATOR != 0, this);
    m_spectrumView = new Host::UI::SpectrumView(m_stateManager, this);
    m_effectsRack = new Host::UI::EffectsRack(m_stateManager, this);

    auto *layout = new QVBoxLayout();
    layout->addWidget(m_connectionToolbar);
    layout->addWidget(m_spectrumView, 1);
    layout->addWidget(m_effectsRack, 1);

    ui->centralwidget->setLayout(layout);
}
