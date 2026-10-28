#ifndef EMBEDDEDDSP_HOST_EFFECTRACK_H
#define EMBEDDEDDSP_HOST_EFFECTRACK_H

#include <QWidget>
#include <QHBoxLayout>
#include "UI/Effects/AbstractRack.h"
#include "UI/Effects/EffectItem.h"
#include "Models/DeviceModel.h"

namespace Host::Core { class AppController; }

namespace Host::UI::Effects {

/// @brief Main container for the dynamic effect widgets.
class EffectRack : public AbstractRack<EffectItem, QHBoxLayout> {
    Q_OBJECT
public:
    explicit EffectRack(Host::Core::AppController& controller, QWidget *parent = nullptr);

public slots:
    /// @brief Triggered when the manifest is ready to rebuild the rack.
    void onManifestReady(const Host::Models::DeviceManifest& manifest);

    /// @brief Clears all widgets and restores initial state.
    void clear();

    void onSlotParameterUpdated(int slotIndex, int paramIndex, float value);
    void onSlotEffectTypeUpdated(int slotIndex, int effectTypeId);
    void onSlotBypassUpdated(int slotIndex, bool bypassed);

signals:
    /// @brief Emitted when all slots have received their initial state during sync.
    void rackReady();

private:
    Host::Core::AppController& m_appController;
    int m_syncedSlotsCount{0};
    int m_expectedSlotsCount{0};
    bool m_isSyncing{false};
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_EFFECTRACK_H
