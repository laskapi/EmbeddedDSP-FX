#ifndef EMBEDDEDDSP_HOST_PARAMETERRACK_H
#define EMBEDDEDDSP_HOST_PARAMETERRACK_H

#include "UI/Effects/AbstractRack.h"
#include "UI/Effects/ParameterItem.h"
#include "Models/DeviceModel.h"

namespace Host::UI::Effects {

/// @brief Container for ParameterItems; owns reserved height for uniform EffectItem slots.
class ParameterRack : public AbstractRack<ParameterItem, QVBoxLayout> {
    Q_OBJECT
public:
    explicit ParameterRack(QWidget *parent = nullptr);

    /// @brief Locks this rack's height to fit @p maxParamCount parameter rows.
    void reserveCapacity(int maxParamCount);

    /// @brief Rebuilds sliders from @p spec without changing reserved height.
    void rebuild(const Host::Models::EffectSpec &spec);

    /// @brief Removes all parameter widgets (empty / None slot).
    void clearParams();

    void updateParam(int paramIdx, float value);

signals:
    void parameterChanged(int paramIdx, float newValue);

private:
    [[nodiscard]] int paramRowHeight() const;

    int m_reservedCount{0};
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_PARAMETERRACK_H
