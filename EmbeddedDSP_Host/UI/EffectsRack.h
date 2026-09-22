#ifndef EMBEDDEDDSP_HOST_EFFECTSRACK_H
#define EMBEDDEDDSP_HOST_EFFECTSRACK_H

#include <QWidget>
#include <QHBoxLayout>
#include <Protocol/ControlPacket.h>
#include "WidgetRack.h"
#include "EffectSpec.h"

class QLabel;

namespace Host {
    class StateManager;
    namespace UI {

class EffectSlot;

/**
 * @brief Main container for the dynamic effect slots.
 * Synchronizes with StateManager to build the GUI from discovery data.
 */
class EffectsRack : public QWidget {
    Q_OBJECT
    using SlotRack = WidgetRack<uint8_t, EffectSlot, QHBoxLayout>;
public:
    explicit EffectsRack(StateManager* manager, QWidget *parent = nullptr);

    /// @brief Syncs the appropriate slot with device state.
    void syncFromDevice(const Protocol::ControlPacket &pkt);

public slots:
    /// @brief Triggered when the manifest is ready to rebuild the rack.
    void onManifestReady();

    /// @brief Clears all slots and restores initial state.
    void clear();

private:
    StateManager* m_manager{nullptr};
    SlotRack* m_rack{nullptr};
    QLabel *m_welcomeLabel{nullptr};
};

} // namespace UI
} // namespace Host

#endif // EMBEDDEDDSP_HOST_EFFECTSRACK_H
