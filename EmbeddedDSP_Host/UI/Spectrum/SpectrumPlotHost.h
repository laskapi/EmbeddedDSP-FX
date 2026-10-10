#ifndef EMBEDDEDDSP_HOST_SPECTRUMPLOTHOST_H
#define EMBEDDEDDSP_HOST_SPECTRUMPLOTHOST_H

#include <QWidget>

class QResizeEvent;

namespace Host::Core { class AppController; }

namespace Host::UI::Spectrum {

class SpectrumPlotView;
class SpectrumPlotWelcomeOverlay;

/// @brief Hosts plot view with a disconnected-state overlay stacked on top.
class SpectrumPlotHost : public QWidget {
    Q_OBJECT
public:
    explicit SpectrumPlotHost(Core::AppController& controller, QWidget* parent = nullptr);

    [[nodiscard]] SpectrumPlotView* plotView() const { return m_spectrumPlotView; }

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    SpectrumPlotView* m_spectrumPlotView{nullptr};
    SpectrumPlotWelcomeOverlay* m_spectrumPlotWelcomeOverlay{nullptr};
};

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUMPLOTHOST_H
