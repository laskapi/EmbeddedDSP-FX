#ifndef EMBEDDEDDSP_HOST_DEVICEINTERFACE_H
#define EMBEDDEDDSP_HOST_DEVICEINTERFACE_H

#include <QSerialPort>
#include <QByteArray>
#include "EffectSpec.h"

namespace Protocol {
    struct AudioFramePacket;
    struct ControlPacket;
}

namespace Host {

/**
 * @brief Low-level communication interface with the DSP hardware.
 * Encapsulates the transport layer (currently Serial/USB CDC).
 */
class DeviceInterface : public QObject
{
    Q_OBJECT

public:
    explicit DeviceInterface(QObject *parent = nullptr);
    ~DeviceInterface() override;
    
    /// @return True if connection is active.
    [[nodiscard]] bool isOpen() const;

    /// @brief Scans for compatible STM32 devices.
    static QString findDevicePort();

public slots:
    /// @brief Connects to the device. If target is empty, attempts auto-discovery.
    bool connect(const QString &target = QString());
    
    /// @brief Disconnects from the device.
    void disconnect();
    
    /// @brief Transmits a control packet.
    bool sendControlPacket(const Protocol::ControlPacket &pkt);

signals:
    void manifestReady(const DeviceManifest &manifest);
    void audioFrameReady(const Protocol::AudioFramePacket &frame);
    void controlPacketReady(const Protocol::ControlPacket &pkt);
    void portStatusChanged(bool isOpen, const QString &portName);
    void errorOccurred(const QString &errorMessage);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    void processRxBuffer();
    DeviceManifest parseManifest(const QString &manifest);
    
    QSerialPort m_serialPort;
    QByteArray m_rxBuffer;
    
    bool m_manifestMode = false;
    QByteArray m_manifestBuffer;
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_DEVICEINTERFACE_H
