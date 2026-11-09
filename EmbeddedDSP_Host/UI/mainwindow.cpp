#include "UI/mainwindow.h"
#include "UI/ConnectionToolbar.h"
#include "UI/Effects/EffectRack.h"
#include "UI/Effects/MasterVolumeControl.h"
#include "UI/Spectrum/SpectrumPanel.h"
#include "Core/AppController.h"
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QSizePolicy>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

namespace {
    constexpr const char* PROP_MAX_HEIGHT = "maximumHeight";
constexpr std::string_view WINDOW_TITLE = "EmbeddedDSP Host - Universal Control";
}

namespace Host::UI {

MainWindow::MainWindow(Core::AppController& controller, QWidget *parent)
    : QMainWindow(parent)
    , m_appController(controller)
{
    setWindowTitle(QString::fromUtf8(WINDOW_TITLE.data(),WINDOW_TITLE.size()));

    setupUiLayout();

    connect(&m_appController, &Core::AppController::connectionChanged, this,
            [this](bool connected, const QString &portName) {
                statusBar()->showMessage(connected ? tr("Connected to %1").arg(portName)
                                                   : tr("Disconnected"));
                if (!connected) {
                    animateFxStrip(false);
                }
            });

    connect(m_effectsRack, &Effects::EffectRack::rackReady, this, [this]() {
        animateFxStrip(true);
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
    m_masterVolume = new Effects::MasterVolumeControl(m_appController, this);

    m_fxStrip = new QWidget(this);
    auto *fxLayout = new QHBoxLayout(m_fxStrip);
    fxLayout->setContentsMargins(0, 0, 0, 0);
    fxLayout->setSpacing(8);
    fxLayout->addWidget(m_effectsRack, 1);
    fxLayout->addWidget(m_masterVolume);

    m_fxStrip->setMaximumHeight(0);
    m_fxStrip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_effectsRack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_masterVolume->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    layout->addWidget(m_connectionToolbar);
    layout->addWidget(m_spectrumPanel, 1);
    layout->addWidget(m_fxStrip);

    setCentralWidget(centralWidget);

    setMinimumSize(1000, 600);
}

void MainWindow::animateFxStrip(bool show) {
    if (show) {
        // Wait for layout to process new items before measuring
        QTimer::singleShot(0, this, [this]() {
            const int rackH = m_effectsRack->sizeHint().height();
            const int masterH = m_masterVolume->sizeHint().height();
            const int endH = std::max(rackH, masterH);

            auto *animation = new QPropertyAnimation(m_fxStrip, PROP_MAX_HEIGHT);
            animation->setDuration(400);
            animation->setEasingCurve(QEasingCurve::InOutQuad);
            animation->setStartValue(0);
            animation->setEndValue(endH);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        });
    } else {
        auto *animation = new QPropertyAnimation(m_fxStrip, PROP_MAX_HEIGHT);
        animation->setDuration(300);
        animation->setEasingCurve(QEasingCurve::InQuad);
        animation->setStartValue(m_fxStrip->height());
        animation->setEndValue(0);

        // Clear rack only AFTER animation finishes
        connect(animation, &QPropertyAnimation::finished, m_effectsRack, &Effects::EffectRack::clear);

        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

} // namespace Host::UI
