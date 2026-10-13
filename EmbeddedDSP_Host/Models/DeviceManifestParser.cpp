#include "DeviceManifestParser.h"

#include <QStringList>

namespace Host::Models {

DeviceManifest parseDeviceManifest(const QString& manifestText) {
    DeviceManifest result;
    const QStringList effectLines = manifestText.split('\n', Qt::SkipEmptyParts);

    for (const QString& effectLine : effectLines) {
        QStringList segments = effectLine.split(';', Qt::SkipEmptyParts);
        if (segments.isEmpty()) {
            continue;
        }

        QStringList header = segments[0].split(':');
        if (header.isEmpty()) {
            continue;
        }

        const QString tag = header[0];

        if (tag == QLatin1String("C")) {
            if (header.size() >= 3 && header[1] == QLatin1String("SLOTS")) {
                result.slotCount = header[2].toInt();
            }
        } else if (tag == QLatin1String("E")) {
            if (header.size() < 3) {
                continue;
            }

            EffectSpec spec;
            spec.id = static_cast<uint8_t>(header[1].toInt());
            spec.name = header[2];

            const auto params = segments.sliced(1);
            for (const QString& param : std::as_const(params)) {
                QStringList p = param.split(':');
                if (p.size() < 5 || p[0] != QLatin1String("P")) {
                    continue;
                }

                spec.params.append({p[1], p[2].toFloat(), p[3].toFloat(), p[4].toFloat()});
            }
            result.availableEffects[spec.id] = spec;
        }
    }

    if (result.slotCount <= 0) {
        result.slotCount = 4;
    }

    return result;
}

} // namespace Host::Models
