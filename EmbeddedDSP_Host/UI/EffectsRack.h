#ifndef EMBEDDEDDSP_HOST_EFFECTSRACK_H
#define EMBEDDEDDSP_HOST_EFFECTSRACK_H

#include <QWidget>
#include <QHBoxLayout>
#include <Protocol/ControlPacket.h>
#include "WidgetRack.h"
#include "EffectSpec.h"

class QLabel;

namespace UI {

class EffectSlot;
class StateManager;

/**
 * @brief UI component representing the entire effects rack.
 * 
 * Dynamically creates and manages multiple EffectSlot widgets based on
 * configuration received from the device via StateManager.
 */
class EffectsRack : public QWidget {
    Q_OBJECT
    using SlotRack = WidgetRack<uint8_t, EffectSlot, QHBoxLayout>;
public:
    /**
     * @brief Constructor for EffectsRack.
     * @param manager Pointer to the central StateManager.
     * @param parent Pointer to the parent QObject.
     */
    explicit EffectsRack(StateManager* manager, QWidget *parent = nullptr);

    /**
     * @brief Synchronizes the rack UI with the device state.
     * @param pkt Control packet received from the device.
     */
    void syncFromDevice(const Protocol::ControlPacket &pkt);

public slots:
    /** @brief Handles manifest processing notification from StateManager. */
    void onManifestProcessed();

    /** @brief Clears all slots and restores the welcome message. */
    void clear();

private:
    StateManager* m_manager{nullptr};
    SlotRack* m_rack{nullptr};
    QLabel *m_welcomeLabel{nullptr};
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_EFFECTSRACK_H
