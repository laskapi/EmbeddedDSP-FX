#ifndef EMBEDDEDDSP_HOST_EFFECTRACK_H
#define EMBEDDEDDSP_HOST_EFFECTRACK_H

#include <QWidget>
#include <QHBoxLayout>
#include "UI/Effects/AbstractRack.h"
#include "UI/Effects/EffectItem.h"
#include "Models/DeviceModel.h"
#include "Protocol/ProtocolCommon.h"

class QPaintEvent;

namespace Host::Core { class AppController; }

namespace Host::UI::Effects {

/// @brief Horizontal effect chain with drag-reorder (arrows painted in gaps).
class EffectRack : public AbstractRack<EffectItem, QHBoxLayout> {
    Q_OBJECT
public:
    explicit EffectRack(Host::Core::AppController& controller, QWidget *parent = nullptr);

public slots:
    /// @brief Rebuild slots when the device manifest arrives.
    void onManifestReady(const Host::Models::DeviceManifest& manifest);

    /// @brief Remove all effect items.
    void clear();

    void onSlotParameterUpdated(int slotIndex, int paramIndex, float value);
    void onSlotEffectTypeUpdated(int slotIndex, int effectTypeId);
    void onSlotBypassUpdated(int slotIndex, bool bypassed);

signals:
    /// @brief Fired once every slot has its initial sync reports.
    void rackReady();

protected:
    void paintEvent(QPaintEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void syncItemLayout();
    void optimisticMove(int from, int to);
    [[nodiscard]] int insertIndexFor(int x) const;
    void showDropIndicator(int insertBefore);
    void hideDropIndicator();
    void refreshDelayExclusivity();

    Host::Core::AppController& m_appController;
    QWidget* m_dropIndicator{nullptr};
    /// @brief ReportEffectType count before rackReady (-1 = not waiting).
    int m_slotsToInit{-1};
    /// @brief Slot that currently holds Delay (-1 = none).
    int m_delayOwner{-1};
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_EFFECTRACK_H
