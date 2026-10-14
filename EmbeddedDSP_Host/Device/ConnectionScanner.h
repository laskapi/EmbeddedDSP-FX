#ifndef EMBEDDEDDSP_HOST_CONNECTION_SCANNER_H
#define EMBEDDEDDSP_HOST_CONNECTION_SCANNER_H

#include "IConnectionScanner.h"

namespace Host::Device {

/// @brief Scans COM ports and always appends the built-in simulator entry.
class ConnectionScanner : public IConnectionScanner {
public:
    [[nodiscard]] std::vector<Host::Models::ConnectionInfo> scan() override;
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_CONNECTION_SCANNER_H
