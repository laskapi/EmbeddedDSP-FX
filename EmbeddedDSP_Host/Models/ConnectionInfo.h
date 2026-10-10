#ifndef EMBEDDEDDSP_HOST_CONNECTIONINFO_H
#define EMBEDDEDDSP_HOST_CONNECTIONINFO_H

#include <QString>

namespace Host::Models {

/// @brief Describes a selectable host-side connection target (port, simulator, etc.).
struct ConnectionInfo {
    QString id;
    QString description;
};

} // namespace Host::Models

#endif // EMBEDDEDDSP_HOST_CONNECTIONINFO_H
