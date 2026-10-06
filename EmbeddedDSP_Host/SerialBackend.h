#ifndef EMBEDDEDDSP_HOST_SERIALBACKEND_H
#define EMBEDDEDDSP_HOST_SERIALBACKEND_H

#include <QSerialPort>
#include <QByteArray>
#include "EffectSpec.h"
#include "IDeviceBackend.h"

namespace Protocol {
    struct AudioFramePacket;
    struct ControlPacket;
}

namespace Host {

/// @brief Hardware implementation of IDeviceBackend using Serial/USB CDC.
class SerialBackend : public IDeviceBackend
{
    Q_OBJECT

public:
    explicit SerialBackend(QObject *parent = nullptr);
    ~SerialBackend() override;
    
    [[nodiscard]] bool isOpen() const override;

public slots:
    bool connect(const QString &target = QString()) override;
    void disconnect() override;
    bool sendControlPacket(const Protocol::ControlPacket &pkt) override;
    [[nodiscard]] std::vector<ConnectionInfo> enumerateConnections() const override;

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    void processRxBuffer();
    DeviceManifest parseManifest(const QString &manifest);
    QString findDevicePort();
    
    QSerialPort m_serialPort;
    QByteArray m_rxBuffer;
    
    bool m_manifestMode = false;
    QByteArray m_manifestBuffer;
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_SERIALBACKEND_H
