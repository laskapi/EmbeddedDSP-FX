#ifndef EMBEDDEDDSP_HOST_DEVICE_MODEL_H
#define EMBEDDEDDSP_HOST_DEVICE_MODEL_H

#include <QString>
#include <QList>
#include <QMap>
#include <cstdint>

namespace Host::Models {

/// @brief One effect knob as described by the device.
struct ParamSpec {
    QString name;
    float min;
    float max;
    float defaultValue;
    QString unit;
};

/// @brief One effect type and its knobs.
struct EffectSpec {
    uint8_t id;
    QString name;
    QList<ParamSpec> params;
};

/// @brief Parsed device manifest (effects + C:SLOTS).
struct DeviceManifest {
    QMap<int, EffectSpec> availableEffects;
    int slotCount{0}; ///< Active rack slots from firmware C:SLOTS.
};

} // namespace Host::Models

#endif // EMBEDDEDDSP_HOST_DEVICE_MODEL_H
