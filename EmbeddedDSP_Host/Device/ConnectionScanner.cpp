#include "ConnectionScanner.h"
#include "DemoConnectionConstants.h"
#include <Protocol/ProtocolCommon.h>
#include <QSerialPortInfo>

namespace Host::Device {

namespace {

/// @brief True when the serial port reports our STM32 USB CDC VID/PID.
bool matchesOurUsbIds(const QSerialPortInfo& info)
{
    return info.hasVendorIdentifier()
           && info.vendorIdentifier() == Protocol::Hardware::USB_VID
           && info.hasProductIdentifier()
           && info.productIdentifier() == Protocol::Hardware::USB_PID;
}
static constexpr int kScanIntervalMs = 2000;

} // namespace

ConnectionScanner::ConnectionScanner(QObject* parent)
    : IConnectionScanner(parent)
    , m_timer(this)
{
    connect(&m_timer, &QTimer::timeout, this, &ConnectionScanner::scan);
}

void ConnectionScanner::start()
{
    m_timer.start(kScanIntervalMs);
    QTimer::singleShot(0, this, &ConnectionScanner::scan);
}

void ConnectionScanner::stop()
{
    m_timer.stop();
}

void ConnectionScanner::scan()
{
    std::vector<Host::Models::ConnectionInfo> connections;
    for (const auto& info : QSerialPortInfo::availablePorts()) {
        if (info.portName().isEmpty()) {
            continue;
        }
        connections.push_back({
            info.portName(),
            info.description(),
            matchesOurUsbIds(info)});
    }
    connections.push_back({
        kDemoConnectionId,
        kDemoConnectionDisplayLabel,
        true});

    emit connectionsUpdated(connections);
}

} // namespace Host::Device
