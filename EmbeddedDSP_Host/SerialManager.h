#ifndef EMBEDDEDDSP_HOST_SERIALMANAGER_H
#define EMBEDDEDDSP_HOST_SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>

#include "Protocol/AudioFramePacket.h"
#include "Protocol/ControlPacket.h"
#include "Protocol/Crc16Calculator.h"
class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager() override;

    bool openPort(const QString &portName, qint32 baudRate = 115200);
    void closePort();
    [[nodiscard]] bool isOpen() const;

    bool sendControlPacket(const ControlPacket::ControlPacket &packet);

signals:
    void audioFrameReceived(const AudioFramePacket &frame);
    void controlPacketReceived(const ControlPacket::ControlPacket &packet);
    void portStatusChanged(bool isOpen, const QString &portName);
    void errorOccurred(const QString &errorMessage);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    void processRxBuffer();
   
    QSerialPort m_serialPort;
    QByteArray m_rxBuffer;
};

#endif // EMBEDDEDDSP_HOST_SERIALMANAGER_H