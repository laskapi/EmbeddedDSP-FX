#include "ConnectionScanner.h"
#include "ConnectionIds.h"
#include <QSerialPortInfo>

namespace Host::Device {

std::vector<Host::Models::ConnectionInfo> ConnectionScanner::scan() {
    std::vector<Host::Models::ConnectionInfo> connections;
    for (const auto& info : QSerialPortInfo::availablePorts()) {
        connections.push_back({info.portName(), info.description()});
    }
    connections.push_back({kDemoConnectionId, kDemoConnectionDescription});
    return connections;
}

} // namespace Host::Device
