#ifndef EMBEDDEDDSP_HOST_SERIALMANAGER_H
#define EMBEDDEDDSP_HOST_SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <Protocol/AudioFramePacket.h>
#include <Protocol/ControlPacket.h>

/**
 * @brief Low-level serial communication manager.
 * 
 * Handles byte-level transmission, packet framing, and SOF detection.
 * Used exclusively by StateManager in the final architecture.
 */
class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager() override;

    /** @brief Opens the serial port with given parameters. */
    bool openPort(const QString &portName, qint32 baudRate = 115200);
    
    /** @brief Closes the serial port. */
    void closePort();
    
    /** @return True if the port is currently open. */
    [[nodiscard]] bool isOpen() const;

    /** @brief Sends a ControlPacket to the device. */
    bool sendControlPacket(const Protocol::ControlPacket &pkt);

    /** @brief Utility to find the first STM32 device port. */
    static QString findDevicePort();

signals:
    /** @brief Emitted when a discovery manifest is received. */
    void manifestReceived(const QString &manifest);
    
    /** @brief Emitted when a valid audio frame is parsed. */
    void audioFrameReceived(const Protocol::AudioFramePacket &frame);
    
    /** @brief Emitted when a valid control report is received. */
    void controlPacketReceived(const Protocol::ControlPacket &pkt);
    
    /** @brief Emitted when the port status changes. */
    void portStatusChanged(bool isOpen, const QString &portName);
    
    /** @brief Emitted on serial errors. */
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
