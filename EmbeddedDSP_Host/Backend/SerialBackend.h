#ifndef EMBEDDEDDSP_HOST_SERIALBACKEND_H
#define EMBEDDEDDSP_HOST_SERIALBACKEND_H

#include <QSerialPort>
#include <QByteArray>
#include "DeviceModel.h"
#include "IDeviceBackend.h"

namespace Protocol {
    struct AudioFramePacket;
    struct ControlPacket;
}

namespace Host::Backend {

/// @brief Hardware implementation of IDeviceBackend using Serial/USB CDC.
class SerialBackend : public IDeviceBackend
{
    Q_OBJECT

public:
    explicit SerialBackend(QObject *parent = nullptr);
    ~SerialBackend() override;

    bool connectToDevice(const QString &target) override;
    void disconnectFromDevice() override;
    bool sendControlPacket(const Protocol::ControlPacket &pkt) override;
    bool isOpen() const override;
    void requestAvailableConnections() override;

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

} // namespace Host::Backend

#endif // EMBEDDEDDSP_HOST_SERIALBACKEND_H
