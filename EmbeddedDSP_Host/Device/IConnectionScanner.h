#ifndef EMBEDDEDDSP_HOST_ICONNECTION_SCANNER_H
#define EMBEDDEDDSP_HOST_ICONNECTION_SCANNER_H

#include <vector>
#include "ConnectionInfo.h"

namespace Host::Device {

/// @brief Discovers selectable connection targets (serial ports + demo).
class IConnectionScanner {
public:
    virtual ~IConnectionScanner() = default;
    [[nodiscard]] virtual std::vector<Host::Models::ConnectionInfo> scan() = 0;
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_ICONNECTION_SCANNER_H
