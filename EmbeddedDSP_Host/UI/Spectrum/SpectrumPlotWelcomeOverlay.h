#ifndef EMBEDDEDDSP_HOST_SPECTRUMPLOTWELCOMEOVERLAY_H
#define EMBEDDEDDSP_HOST_SPECTRUMPLOTWELCOMEOVERLAY_H

#include <QWidget>

namespace Host::Core { class AppController; }

namespace Host::UI::Spectrum {

/// @brief Empty-state overlay shown over the spectrum plot when disconnected.
class SpectrumPlotWelcomeOverlay : public QWidget {
    Q_OBJECT
public:
    explicit SpectrumPlotWelcomeOverlay(Core::AppController& controller, QWidget* parent = nullptr);

private:
    Core::AppController& m_appController;
};

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUMPLOTWELCOMEOVERLAY_H
