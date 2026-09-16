#ifndef EMBEDDEDDSP_HOST_EFFECTSPEC_H
#define EMBEDDEDDSP_HOST_EFFECTSPEC_H

#include <QString>
#include <QList>
#include <cstdint>

namespace Host {

/**
 * @brief Technical specification for a single effect parameter.
 */
struct ParamSpec {
    QString name;
    float min;
    float max;
    float defaultValue;
};

/**
 * @brief Technical specification for an effect, describing its capabilities.
 */
struct EffectSpec {
    uint8_t id;
    QString name;
    QList<ParamSpec> params;
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_EFFECTSPEC_H
