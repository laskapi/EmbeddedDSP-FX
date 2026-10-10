#ifndef EMBEDDEDDSP_HOST_PARAMETERRACK_H
#define EMBEDDEDDSP_HOST_PARAMETERRACK_H

#include "AbstractRack.h"
#include "ParameterItem.h"
#include "EffectSpec.h"

namespace Host::UI::Effects {

/// @brief Container for ParameterItems.
class ParameterRack : public AbstractRack<ParameterItem, QVBoxLayout> {
    Q_OBJECT
public:
    explicit ParameterRack(const Host::Models::EffectSpec &spec, QWidget *parent = nullptr);

    void updateParam(int paramIdx, float value);

signals:
    void parameterChanged(int paramIdx, float newValue);
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_PARAMETERRACK_H
