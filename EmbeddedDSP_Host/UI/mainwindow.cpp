#include "UI/mainwindow.h"
#include "UI/ConnectionToolbar.h"
#include "UI/Effects/EffectItem.h"
#include "UI/Effects/EffectRack.h"
#include "UI/Effects/MasterVolumeControl.h"
#include "UI/Effects/PresetBar.h"
#include "UI/Spectrum/SpectrumPanel.h"
#include "Core/AppController.h"
#include "Protocol/ProtocolCommon.h"

#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QSizePolicy>
#include <QStatusBar>
#include <QVBoxLayout>
#include <algorithm>

namespace {
constexpr const char* PROP_MAX_HEIGHT = "maximumHeight";
constexpr std::string_view WINDOW_TITLE = "EmbeddedDSP Host - Universal Control";
constexpr int kMinWindowWidth = 1000;
constexpr int kSpectrumMinHeight = 150;
}

namespace Host::UI {

MainWindow::MainWindow(Core::AppController& controller, QWidget *parent)
    : QMainWindow(parent)
    , m_appController(controller)
{
    setWindowTitle(QString::fromUtf8(WINDOW_TITLE.data(), WINDOW_TITLE.size()));

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

    connect(m_effectsRack, &Effects::EffectRack::eqCurvesChanged,
            m_spectrumPanel, &Spectrum::SpectrumPanel::setEqCurves);

    connect(&m_appController, &Core::AppController::errorOccurred, this,
            [this](const QString &errorMessage) {
                statusBar()->showMessage(tr("Error: %1").arg(errorMessage), 5000);
            });
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUiLayout() {
    m_connectionToolbar = new ConnectionToolbar(m_appController, this);
    m_presetBar = new Effects::PresetBar(m_appController, this);
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

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    layout->addWidget(m_connectionToolbar);
    layout->addWidget(m_spectrumPanel, 1);
    layout->addWidget(m_presetBar);
    layout->addWidget(m_fxStrip);

    setCentralWidget(centralWidget);
    setMinimumWidth(kMinWindowWidth);

    m_presetBar->setVisible(false);

    m_fxStripHeight = measureFxStripHeight();
    ensureWindowFitsFxStrip(m_fxStripHeight);
}

int MainWindow::measureFxStripHeight() {
    auto* probe = new Effects::EffectItem(0, this);
    probe->setAttribute(Qt::WA_DontShowOnScreen);
    probe->lockHeightForParamCapacity(Protocol::kMaxEffectParams);
    const int slotH = probe->sizeHint().height();
    delete probe;

    m_masterVolume->setFixedHeight(slotH);
    return slotH;
}

void MainWindow::ensureWindowFitsFxStrip(int fxStripHeight) {
    auto* lay = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    const QMargins margins = lay ? lay->contentsMargins() : QMargins{};
    const int spacing = lay ? lay->spacing() : 0;
    // toolbar + spectrum + presets + fxStrip => 3 gaps between 4 rows
    constexpr int kRowGaps = 3;

    const int chrome =
        m_connectionToolbar->sizeHint().height()
        + m_presetBar->sizeHint().height()
        + statusBar()->sizeHint().height()
        + margins.top() + margins.bottom()
        + spacing * kRowGaps;

    const int spectrumMin = std::max(kSpectrumMinHeight, m_spectrumPanel->minimumSizeHint().height());
    const int needed = chrome + spectrumMin + std::max(0, fxStripHeight);
    const int newMin = std::max(minimumHeight(), needed);
    setMinimumHeight(newMin);
    if (height() < newMin) {
        resize(width(), newMin);
    }
}

void MainWindow::animateFxStrip(bool show) {
    if (show) {
        // Intro only when strip is collapsed (avoid re-animating on accidental rackReady).
        if (m_fxStrip->maximumHeight() > 0) {
            return;
        }

        m_presetBar->setVisible(true);

        auto *animation = new QPropertyAnimation(m_fxStrip, PROP_MAX_HEIGHT);
        animation->setDuration(400);
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->setStartValue(0);
        animation->setEndValue(m_fxStripHeight);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        m_presetBar->setVisible(false);

        auto *animation = new QPropertyAnimation(m_fxStrip, PROP_MAX_HEIGHT);
        animation->setDuration(300);
        animation->setEasingCurve(QEasingCurve::InQuad);
        animation->setStartValue(m_fxStrip->height());
        animation->setEndValue(0);

        connect(animation, &QPropertyAnimation::finished, m_effectsRack, &Effects::EffectRack::clear);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

} // namespace Host::UI
