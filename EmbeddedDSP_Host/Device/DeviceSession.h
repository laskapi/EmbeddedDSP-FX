#ifndef EMBEDDEDDSP_HOST_DEVICE_SESSION_H
#define EMBEDDEDDSP_HOST_DEVICE_SESSION_H

#include <QObject>
#include <QString>
#include <memory>

namespace Host::Core { class WorkerThread; }

namespace Host::Device {

class IDevice;
class IDeviceFactory;

/// @brief Owns the active IDevice on the host worker thread and replaces it safely.
class DeviceSession : public QObject {
    Q_OBJECT
public:
    DeviceSession(Host::Core::WorkerThread& worker,
                  std::unique_ptr<IDeviceFactory> factory,
                  QObject* parent = nullptr);
    ~DeviceSession() override;

    /// @brief Creates a device for @p connectionId, adopts it on the worker, and opens it.
    void open(const QString& connectionId);

    [[nodiscard]] IDevice* device() const { return m_device; }

signals:
    void deviceReady(Host::Device::IDevice* device);

private:
    void replace(std::unique_ptr<IDevice> device);
    void clearCurrentDevice();

    Host::Core::WorkerThread* m_worker{nullptr};
    std::unique_ptr<IDeviceFactory> m_factory;
    IDevice* m_device{nullptr};
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_DEVICE_SESSION_H
