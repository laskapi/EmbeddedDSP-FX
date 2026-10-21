#ifndef EMBEDDEDDSP_HOST_ICONNECTION_SCANNER_H
#define EMBEDDEDDSP_HOST_ICONNECTION_SCANNER_H

#include "ConnectionInfo.h"
#include <QObject>
#include <vector>

namespace Host::Device {

/// @brief Discovers selectable connection targets (serial ports + demo).
class IConnectionScanner : public QObject {
    Q_OBJECT
public:
    explicit IConnectionScanner(QObject* parent = nullptr) : QObject(parent) {}
    ~IConnectionScanner() override = default;

    virtual void start() = 0;
    virtual void stop() = 0;

signals:
    void connectionsUpdated(const std::vector<Host::Models::ConnectionInfo>& connections);
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_ICONNECTION_SCANNER_H
