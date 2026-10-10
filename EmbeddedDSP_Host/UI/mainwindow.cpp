#include "mainwindow.h"
#include "ConnectionToolbar.h"
#include "EffectRack.h"
#include "SpectrumPanel.h"
#include "AppController.h"
#include <QVBoxLayout>
#include <QStatusBar>
#include <QPropertyAnimation>
#include <QTimer>

namespace {
    constexpr const char* PROP_MAX_HEIGHT = "maximumHeight";
}

namespace Host::UI {

MainWindow::MainWindow(Core::AppController& controller, QWidget *parent)
    : QMainWindow(parent)
    , m_appController(controller)
{
    setWindowTitle(QStringLiteral("EmbeddedDSP Host - Universal Control"));

    setupUiLayout();

    connect(&m_appController, &Core::AppController::connectionChanged, this,
            [this](bool connected, const QString &portName) {
                statusBar()->showMessage(connected ? tr("Connected to %1").arg(portName)
                                                   : tr("Disconnected"));
                if (!connected) {
                    animateRack(false);
                }
            });

    connect(m_effectsRack, &Effects::EffectRack::rackReady, this, [this]() {
        animateRack(true);
    });

    connect(&m_appController, &Core::AppController::errorOccurred, this,
            [this](const QString &errorMessage) {
                statusBar()->showMessage(tr("Error: %1").arg(errorMessage), 5000);
            });
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUiLayout() {
    m_connectionToolbar = new ConnectionToolbar(m_appController, this);
    m_spectrumPanel = new Spectrum::SpectrumPanel(m_appController, this);
    m_effectsRack = new Effects::EffectRack(m_appController, this);

    m_effectsRack->setMaximumHeight(0);
    m_effectsRack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    layout->addWidget(m_connectionToolbar);
    layout->addWidget(m_spectrumPanel, 1);
    layout->addWidget(m_effectsRack, 0);

    setCentralWidget(centralWidget);

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
        connect(animation, &QPropertyAnimation::finished, m_effectsRack, &Effects::EffectRack::clear);

        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

} // namespace Host::UI
