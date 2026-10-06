#ifndef EMBEDDEDDSP_HOST_PARAMETERRACK_H
#define EMBEDDEDDSP_HOST_PARAMETERRACK_H

#include "AbstractRack.h"
#include "ParameterItem.h"
#include "EffectSpec.h"

namespace Host::UI {

/// @brief Container for ParameterItems.
class ParameterRack : public AbstractRack<ParameterItem, QVBoxLayout> {
    Q_OBJECT
public:
    explicit ParameterRack(const EffectSpec &spec, QWidget *parent = nullptr);

    void updateParam(int paramIdx, float value);

signals:
    void parameterChanged(int paramIdx, float newValue);
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_PARAMETERRACK_H
