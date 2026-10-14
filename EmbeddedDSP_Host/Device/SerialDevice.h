#ifndef EMBEDDEDDSP_HOST_SERIAL_DEVICE_H
#define EMBEDDEDDSP_HOST_SERIAL_DEVICE_H

#include <QSerialPort>
#include <QByteArray>
#include "IDevice.h"

namespace Protocol {
struct AudioFramePacket;
struct ControlPacket;
}

namespace Host::Device {

/// @brief Hardware IDevice using Serial/USB CDC.
class SerialDevice : public IDevice {
    Q_OBJECT
public:
    explicit SerialDevice(QObject* parent = nullptr);
    ~SerialDevice() override;

    bool connectToDevice(const QString& target) override;
    void disconnectFromDevice() override;
    bool sendControlPacket(const Protocol::ControlPacket& pkt) override;
    bool isOpen() const override;

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    void processRxBuffer();
    QString findDevicePort();

    QSerialPort m_serialPort;
    QByteArray m_rxBuffer;
    bool m_manifestMode = false;
    QByteArray m_manifestBuffer;
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_SERIAL_DEVICE_H
