#ifndef EMBEDDEDDSP_HOST_EFFECTSPEC_H
#define EMBEDDEDDSP_HOST_EFFECTSPEC_H

#include <QString>
#include <QList>
#include <QMap>
#include <cstdint>

namespace Host::Models {
    
/// @brief Specification for a single effect parameter.
struct ParamSpec {
    QString name;
    float min;
    float max;
    float defaultValue;
};

/// @brief Specification for an effect and its parameters.
struct EffectSpec {
    uint8_t id;
    QString name;
    QList<ParamSpec> params;
};

/// @brief Full device capabilities received during handshake.
struct DeviceManifest {
    QMap<int, EffectSpec> availableEffects;
    int slotCount{0};
};

} // namespace Host::Models

#endif // EMBEDDEDDSP_HOST_EFFECTSPEC_H
