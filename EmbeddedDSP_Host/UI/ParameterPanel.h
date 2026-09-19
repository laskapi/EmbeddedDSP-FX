#ifndef EMBEDDEDDSP_HOST_PARAMETERPANEL_H
#define EMBEDDEDDSP_HOST_PARAMETERPANEL_H

#include <QWidget>
#include "WidgetRack.h"
#include "EffectSpec.h"

namespace UI {

class ParamControl;
class StateManager;

/**
 * @brief UI panel that displays and manages a set of effect parameters.
 * 
 * Uses WidgetRack to organize multiple ParamControl widgets and handles
 * parameter change requests via StateManager.
 */
class ParameterPanel : public QWidget {
    Q_OBJECT
    using ControlRack = WidgetRack<uint8_t, ParamControl>;
public:
    /**
     * @brief Constructor for ParameterPanel.
     * @param spec Specification of the effect to build UI for.
     * @param slotId Index of the effect slot this panel belongs to.
     * @param manager Pointer to the central StateManager.
     * @param parent Pointer to the parent QObject.
     */
    explicit ParameterPanel(const Host::EffectSpec &spec, uint8_t slotId, StateManager* manager, QWidget *parent = nullptr);

    /**
     * @brief Updates a parameter's UI value without triggering a request to the device.
     * @param paramId Index of the parameter.
     * @param value New value.
     */
    void updateParam(uint8_t paramId, float value);

private slots:
    /** @brief Handles value change notifications from child controls. */
    void onParamChanged(uint8_t paramId, float newValue);

private:
    ControlRack* m_rack{nullptr};
    StateManager* m_manager{nullptr};
    uint8_t m_slotId;
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_PARAMETERPANEL_H
