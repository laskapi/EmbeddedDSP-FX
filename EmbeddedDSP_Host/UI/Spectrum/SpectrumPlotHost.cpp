#include "SpectrumPlotHost.h"
#include "SpectrumPlotView.h"
#include "SpectrumPlotWelcomeOverlay.h"
#include <QResizeEvent>
#include <QVBoxLayout>

namespace Host::UI::Spectrum {

SpectrumPlotHost::SpectrumPlotHost(Core::AppController& controller, QWidget* parent)
    : QWidget(parent)
{
    auto* plotLayout = new QVBoxLayout(this);
    plotLayout->setContentsMargins(0, 0, 0, 0);
    plotLayout->setSpacing(0);

    m_spectrumPlotView = new SpectrumPlotView(controller, this);
    plotLayout->addWidget(m_spectrumPlotView);

    m_spectrumPlotWelcomeOverlay = new SpectrumPlotWelcomeOverlay(controller, this);
    m_spectrumPlotWelcomeOverlay->setGeometry(rect());
    m_spectrumPlotWelcomeOverlay->raise();
}

void SpectrumPlotHost::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    m_spectrumPlotWelcomeOverlay->setGeometry(rect());
    m_spectrumPlotWelcomeOverlay->raise();
}

} // namespace Host::UI::Spectrum
