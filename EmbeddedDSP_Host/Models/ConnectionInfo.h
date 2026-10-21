#ifndef EMBEDDEDDSP_HOST_CONNECTIONINFO_H
#define EMBEDDEDDSP_HOST_CONNECTIONINFO_H

#include <QString>

namespace Host::Models {

/// @brief Describes a selectable host-side connection target..
struct ConnectionInfo {
    QString id;
    QString description;
    /// @brief When false, UI shows the entry greyed out (e.g. foreign USB serial).
    bool selectable{true};
};

} // namespace Host::Models

#endif // EMBEDDEDDSP_HOST_CONNECTIONINFO_H
