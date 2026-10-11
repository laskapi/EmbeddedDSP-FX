#include "SpectrumPlotWelcomeOverlay.h"
#include "AppController.h"
#include "UiObjectNames.h"
#include <QLabel>
#include <QVBoxLayout>

namespace {

constexpr const char kSpectrumWelcomeMessage[] =
    "Please connect your EmbeddedDSP device...";

} // namespace

namespace Host::UI::Spectrum {

SpectrumPlotWelcomeOverlay::SpectrumPlotWelcomeOverlay(Core::AppController& controller, QWidget* parent)
    : QWidget(parent)
    , m_appController(controller)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAutoFillBackground(false);

    auto* label = new QLabel(tr(kSpectrumWelcomeMessage), this);
    label->setObjectName(Host::UI::UiObjectNames::kSpectrumWelcomeLabel);
    label->setAlignment(Qt::AlignCenter);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(label);

    connect(&m_appController, &Core::AppController::connectionChanged,
            this, [this](bool connected) {
                setVisible(!connected);
            });
}

} // namespace Host::UI::Spectrum
