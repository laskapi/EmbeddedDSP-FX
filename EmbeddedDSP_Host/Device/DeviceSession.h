#ifndef EMBEDDEDDSP_HOST_DEVICE_SESSION_H
#define EMBEDDEDDSP_HOST_DEVICE_SESSION_H

#include <QObject>
#include <memory>

class QThread;

namespace Host::Device {

class IDevice;

/// @brief Owns the active IDevice on the worker thread and replaces it safely.
class DeviceSession : public QObject {
    Q_OBJECT
public:
    explicit DeviceSession(QThread* workerThread, QObject* parent = nullptr);
    ~DeviceSession() override;

    /// @brief Tears down the previous device (if any) and activates @p device on the worker thread.
    void replace(std::unique_ptr<IDevice> device);

    [[nodiscard]] IDevice* device() const { return m_device; }

signals:
    void deviceReady(Host::Device::IDevice* device);

private:
    void clearCurrentDevice();

    QThread* m_workerThread{nullptr};
    IDevice* m_device{nullptr};
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_DEVICE_SESSION_H
