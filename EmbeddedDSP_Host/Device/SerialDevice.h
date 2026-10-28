#ifndef EMBEDDEDDSP_HOST_SERIAL_DEVICE_H
#define EMBEDDEDDSP_HOST_SERIAL_DEVICE_H

#include <QSerialPort>
#include <QByteArray>
#include "Device/IDevice.h"

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

    bool open(const QString& connectionId) override;
    void close() override;
    bool sendControlPacket(const Protocol::ControlPacket& pkt) override;

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    void processRxBuffer();

    QSerialPort m_serialPort;
    QByteArray m_rxBuffer;
    bool m_manifestMode = false;
    QByteArray m_manifestBuffer;
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_SERIAL_DEVICE_H
