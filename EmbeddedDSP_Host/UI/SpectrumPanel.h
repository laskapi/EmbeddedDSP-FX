#ifndef EMBEDDEDDSP_HOST_SPECTRUMPANEL_H
#define EMBEDDEDDSP_HOST_SPECTRUMPANEL_H

#include <QFrame>
#include <memory>

class QSlider;
class QLabel;
class QResizeEvent;

namespace Host { class AppController; }

namespace Host::UI {

class SpectrumView;

/**
 * @brief Container that combines SpectrumView with control widgets (Zoom/Sensitivity).
 */
class SpectrumPanel : public QFrame {
    Q_OBJECT
public:
    /// @brief Main spectrum panel constructor.
    explicit SpectrumPanel(std::shared_ptr<AppController> controller, QWidget *parent = nullptr);
    ~SpectrumPanel() override = default;

protected:
    /// @brief Ensures perfect symmetry between side columns.
    void resizeEvent(QResizeEvent *event) override;

private:
    std::shared_ptr<AppController> m_appController;
    SpectrumView* m_spectrumView{nullptr};
    QWidget* m_leftColumn{nullptr};
    QWidget* m_rightColumn{nullptr};
    QSlider* m_dbSlider{nullptr};
    QLabel* m_zoomIcon{nullptr};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_SPECTRUMPANEL_H
