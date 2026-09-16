#ifndef EMBEDDEDDSP_HOST_SERIALMANAGER_H
#define EMBEDDEDDSP_HOST_SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <Protocol/AudioFramePacket.h>
#include <Protocol/ControlPacket.h>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager() override;

    bool openPort(const QString &portName, qint32 baudRate = 115200);
    void closePort();
    [[nodiscard]] bool isOpen() const;

    bool sendControlPacket(const Protocol::ControlPacket &pkt);
    static QString findDevicePort();

signals:
    void manifestReceived(const QString &manifest);
    void audioFrameReceived(const Protocol::AudioFramePacket &frame);
    void controlPacketReceived(const Protocol::ControlPacket &pkt);
    void portStatusChanged(bool isOpen, const QString &portName);
    void errorOccurred(const QString &errorMessage);

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

#endif // EMBEDDEDDSP_HOST_SERIALMANAGER_H
