#ifndef EMBEDDEDDSP_HOST_PARAMETERPANEL_H
#define EMBEDDEDDSP_HOST_PARAMETERPANEL_H

#include <QWidget>
#include "WidgetRack.h"
#include "EffectSpec.h"

namespace Host {
    class StateManager;
    namespace UI {

class ParamControl;

/**
 * @brief UI panel displaying effect-specific parameter controls.
 * Handles parameter change requests via StateManager.
 */
class ParameterPanel : public QWidget {
    Q_OBJECT
    using ControlRack = WidgetRack<uint8_t, ParamControl>;
public:
    explicit ParameterPanel(const EffectSpec &spec, uint8_t slotId, StateManager* manager, QWidget *parent = nullptr);

    /// @brief Updates a parameter's UI value without triggering requests.
    void updateParam(uint8_t paramId, float value);

private slots:
    /// @brief Handles user-initiated parameter changes.
    void onParamChanged(uint8_t paramId, float newValue);

private:
    ControlRack* m_rack{nullptr};
    StateManager* m_manager{nullptr};
    uint8_t m_slotId;
};

} // namespace UI
} // namespace Host

#endif // EMBEDDEDDSP_HOST_PARAMETERPANEL_H
