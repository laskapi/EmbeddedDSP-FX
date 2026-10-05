#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UI/ConnectionToolbar.h"
#include "UI/EffectRack.h"
#include "UI/Spectrum/SpectrumPanel.h"
#include "AppController.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QStatusBar>
#include <QPropertyAnimation>
#include <QTimer>

namespace {
    constexpr const char* PROP_MAX_HEIGHT = "maximumHeight";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("EmbeddedDSP Host - Universal Control"));

    m_appController = std::make_shared<Host::AppController>();
    
    setupUiLayout();

    connect(m_appController.get(), &Host::AppController::connectionChanged, this, [this](bool connected, const QString &portName) {
        statusBar()->showMessage(connected ? tr("Connected to %1").arg(portName) : tr("Disconnected"));
        m_spectrumPanel->setWelcomeVisible(!connected);
        animateRack(connected);
    });

    connect(m_appController.get(), &Host::AppController::errorOccurred, this, [this](const QString &errorMessage) {
        statusBar()->showMessage(tr("Error: %1").arg(errorMessage), 5000);
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUiLayout() {
    m_connectionToolbar = new Host::UI::ConnectionToolbar(m_appController, this);
    m_spectrumPanel = new Host::UI::Spectrum::SpectrumPanel(m_appController, this);
    m_effectsRack = new Host::UI::EffectRack(m_appController, this);

    m_effectsRack->setMaximumHeight(0);
    m_effectsRack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *layout = new QVBoxLayout();
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    
    layout->addWidget(m_connectionToolbar);
    layout->addWidget(m_spectrumPanel, 1); 
    layout->addWidget(m_effectsRack, 0);   
    
    ui->centralwidget->setLayout(layout);
    
    setMinimumSize(1000, 600);
}

void MainWindow::animateRack(bool show) {
    if (show) {
        // Wait for layout to process new items before measuring
        QTimer::singleShot(0, this, [this]() {
            auto *animation = new QPropertyAnimation(m_effectsRack, PROP_MAX_HEIGHT);
            animation->setDuration(400);
            animation->setEasingCurve(QEasingCurve::InOutQuad);
            animation->setStartValue(0);
            animation->setEndValue(m_effectsRack->sizeHint().height());
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        });
    } else {
        auto *animation = new QPropertyAnimation(m_effectsRack, PROP_MAX_HEIGHT);
        animation->setDuration(300);
        animation->setEasingCurve(QEasingCurve::InQuad);
        animation->setStartValue(m_effectsRack->height());
        animation->setEndValue(0);
        
        // Clear rack only AFTER animation finishes
        connect(animation, &QPropertyAnimation::finished, m_effectsRack, &Host::UI::EffectRack::clear);
        
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
