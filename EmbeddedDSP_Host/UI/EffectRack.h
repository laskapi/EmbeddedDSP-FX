#ifndef EMBEDDEDDSP_HOST_EFFECTRACK_H
#define EMBEDDEDDSP_HOST_EFFECTRACK_H

#include <memory>
#include <QWidget>
#include <QHBoxLayout>
#include <Protocol/ControlPacket.h>
#include "AbstractRack.h"
#include "EffectItem.h"
#include "EffectSpec.h"

class QLabel;

namespace Host { class AppController; }

namespace Host::UI {

/**
 * @brief Main container for the dynamic effect widgets.
 */
class EffectRack : public AbstractRack<EffectItem, QHBoxLayout> {
    Q_OBJECT
public:
    explicit EffectRack(std::shared_ptr<AppController> controller, QWidget *parent = nullptr);

    /// @brief Syncs the appropriate widget with device state.
    void syncFromDevice(const Protocol::ControlPacket &pkt);

public slots:
    /// @brief Triggered when the manifest is ready to rebuild the rack.
    void onManifestReady(const Host::DeviceManifest& manifest);

    /// @brief Clears all widgets and restores initial state.
    void clear();

private:
    std::shared_ptr<AppController> m_appController;
    QLabel *m_welcomeLabel{nullptr};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_EFFECTRACK_H
