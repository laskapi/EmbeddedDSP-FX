#ifndef EMBEDDEDDSP_HOST_PARAMETERRACK_H
#define EMBEDDEDDSP_HOST_PARAMETERRACK_H

#include "SynchronizedRack.h"
#include "ParameterWidget.h"
#include "EffectSpec.h"

namespace Host::UI {

/**
 * @brief Container for ParameterWidgets.
 */
class ParameterRack : public SynchronizedRack<ParameterWidget> {
    Q_OBJECT
public:
    explicit ParameterRack(const EffectSpec &spec, QWidget *parent = nullptr);

    void updateParam(int paramIdx, float value);

signals:
    void parameterChanged(int paramIdx, float newValue);
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_PARAMETERRACK_H
