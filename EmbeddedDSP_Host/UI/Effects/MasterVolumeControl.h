#ifndef EMBEDDEDDSP_HOST_MASTER_VOLUME_CONTROL_H
#define EMBEDDEDDSP_HOST_MASTER_VOLUME_CONTROL_H

#include <QWidget>

class QLabel;
class QSlider;

namespace Host::Core { class AppController; }

namespace Host::UI::Effects {

/// @brief Vertical master-output gain control (0..1), synced via Set/ReportMasterVolume.
class MasterVolumeControl : public QWidget {
    Q_OBJECT
public:
    explicit MasterVolumeControl(Host::Core::AppController& controller, QWidget* parent = nullptr);

public slots:
    void applyMasterVolume(float gain);

private slots:
    void onSliderMoved(int pos);

private:
    static constexpr int kSliderResolution = 1000;

    void setSliderFromGain(float gain);

    Host::Core::AppController& m_appController;
    QLabel* m_titleLabel{nullptr};
    QSlider* m_slider{nullptr};
    QLabel* m_valueLabel{nullptr};
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_MASTER_VOLUME_CONTROL_H
