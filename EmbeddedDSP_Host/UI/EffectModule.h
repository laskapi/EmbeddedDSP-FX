#ifndef EMBEDDEDDSP_HOST_EFFECTMODULE_H
#define EMBEDDEDDSP_HOST_EFFECTMODULE_H

#include <QWidget>
#include "WidgetRack.h"
#include "../EffectSpec.h"

namespace UI {

class ParamControl;

class EffectModule : public QWidget {
    Q_OBJECT
    using ControlRack = WidgetRack<uint8_t, ParamControl>;
public:
    explicit EffectModule(const Host::EffectSpec &spec, QWidget *parent = nullptr);
    void updateParam(uint8_t paramId, float value);

signals:
    void paramChanged(uint8_t paramId, float newValue);

private:
    ControlRack* m_rack;
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_EFFECTMODULE_H
