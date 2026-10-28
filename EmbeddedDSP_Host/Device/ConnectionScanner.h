#ifndef EMBEDDEDDSP_HOST_CONNECTION_SCANNER_H
#define EMBEDDEDDSP_HOST_CONNECTION_SCANNER_H

#include "Device/IConnectionScanner.h"
#include <QTimer>

namespace Host::Device {

/// @brief Scans COM ports on a timer and always appends the built-in simulator entry.
class ConnectionScanner : public IConnectionScanner {
    Q_OBJECT
public:
    explicit ConnectionScanner(QObject* parent = nullptr);
    void start() override;
    void stop() override;

private slots:
    void scan();

private:

    QTimer m_timer;
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_CONNECTION_SCANNER_H
