#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UI/ConnectionToolbar.h"
#include "UI/EffectRack.h"
#include "UI/Spectrum/SpectrumPanel.h"
#include "AppController.h"
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

    m_appController = std::make_shared<Host::AppController>();

    setupUiLayout();

    // Device events wiring
    connect(m_appController.get(), &Host::AppController::connectionChanged, this, [this](bool connected, const QString &portName) {
        statusBar()->showMessage(connected ? tr("Connected to %1").arg(portName) : tr("Disconnected"));
    });

    connect(m_appController.get(), &Host::AppController::errorOccurred, this, [this](const QString &errorMessage) {
        statusBar()->showMessage(tr("Error: %1").arg(errorMessage), 5000);
    });

    // Initial auto-connect
    m_appController->connect();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUiLayout() {
    m_connectionToolbar = new Host::UI::ConnectionToolbar(m_appController, EMBEDDED_DSP_HOST_ENABLE_SIMULATOR != 0, this);
    m_spectrumPanel = new Host::UI::Spectrum::SpectrumPanel(m_appController, this);
    m_effectsRack = new Host::UI::EffectRack(m_appController, this);

    auto *layout = new QVBoxLayout();
    layout->addWidget(m_connectionToolbar);
    layout->addWidget(m_spectrumPanel, 1);
    layout->addWidget(m_effectsRack, 1);

    ui->centralwidget->setLayout(layout);
}
