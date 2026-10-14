#ifndef EMBEDDEDDSP_HOST_DEMO_DEVICE_H
#define EMBEDDEDDSP_HOST_DEMO_DEVICE_H

#include "IDevice.h"
#include <memory>

namespace Host::Device {

namespace Demo { class DemoAudioSimulator; }

/// @brief Local simulated device for running the host without hardware.
class DemoDevice : public IDevice {
    Q_OBJECT
public:
    explicit DemoDevice(QObject* parent = nullptr);
    ~DemoDevice() override;

    bool connectToDevice(const QString& target) override;
    void disconnectFromDevice() override;
    bool sendControlPacket(const Protocol::ControlPacket& pkt) override;
    bool isOpen() const override;

private:
    std::unique_ptr<Demo::DemoAudioSimulator> m_simulator;
    bool m_isOpen{false};
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_DEMO_DEVICE_H
