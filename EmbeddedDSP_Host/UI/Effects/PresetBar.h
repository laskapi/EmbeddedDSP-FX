#ifndef EMBEDDEDDSP_HOST_PRESET_BAR_H
#define EMBEDDEDDSP_HOST_PRESET_BAR_H

#include "Protocol/ProtocolCommon.h"
#include <QWidget>
#include <array>

class QPushButton;

namespace Host::Core { class AppController; }

namespace Host::UI::Effects {

/// @brief User preset bank 0..9 + Save (explicit persist, no auto-save).
class PresetBar : public QWidget {
    Q_OBJECT
public:
    explicit PresetBar(Host::Core::AppController& controller, QWidget* parent = nullptr);

private slots:
    void onActivePresetUpdated(int presetId);
    void onConnectionChanged(bool connected, const QString& portName);
    void onPresetClicked(int presetId);
    void onSaveClicked();

private:
    void setSelectedPreset(int presetId);
    void setEnabledState(bool enabled);

    Host::Core::AppController& m_appController;
    std::array<QPushButton*, Protocol::kUserPresetCount> m_presetButtons{};
    QPushButton* m_saveButton{nullptr};
    int m_selectedPreset{0};
    bool m_connected{false};
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_PRESET_BAR_H
