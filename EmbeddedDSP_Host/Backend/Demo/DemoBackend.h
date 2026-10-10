#ifndef EMBEDDEDDSP_HOST_DEMOBACKEND_H
#define EMBEDDEDDSP_HOST_DEMOBACKEND_H

#include "IDeviceBackend.h"
#include <memory>

namespace Host::Backend {

namespace Demo { class DemoAudioSimulator; }

/// @brief Backend that simulates a hardware device locally.
class DemoBackend : public IDeviceBackend {
    Q_OBJECT
public:
    explicit DemoBackend(QObject *parent = nullptr);
    ~DemoBackend() override;

    bool connectToDevice(const QString &target) override;
    void disconnectFromDevice() override;
    bool sendControlPacket(const Protocol::ControlPacket &pkt) override;
    bool isOpen() const override;
    std::vector<Host::Models::ConnectionInfo> availableConnections() const override;

private:
    std::unique_ptr<Demo::DemoAudioSimulator> m_simulator;
    bool m_isOpen{false};
};

} // namespace Host::Backend

#endif // EMBEDDEDDSP_HOST_DEMOBACKEND_H
