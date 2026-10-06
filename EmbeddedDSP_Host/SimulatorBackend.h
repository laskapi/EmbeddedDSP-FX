#ifndef EMBEDDEDDSP_HOST_SIMULATORBACKEND_H
#define EMBEDDEDDSP_HOST_SIMULATORBACKEND_H

#include "IDeviceBackend.h"
#include <memory>

namespace Host {

class AudioFrameSimulator;

/// @brief Backend that simulates a hardware device locally.
class SimulatorBackend : public IDeviceBackend {
    Q_OBJECT
public:
    explicit SimulatorBackend(QObject *parent = nullptr);
    ~SimulatorBackend() override;

    bool connect(const QString &target) override;
    void disconnect() override;
    bool sendControlPacket(const Protocol::ControlPacket &pkt) override;
    bool isOpen() const override;
    std::vector<ConnectionInfo> enumerateConnections() const override;

private:
    std::unique_ptr<AudioFrameSimulator> m_simulator;
    bool m_isOpen{false};
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_SIMULATORBACKEND_H
