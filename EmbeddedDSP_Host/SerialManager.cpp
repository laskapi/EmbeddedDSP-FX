#include "SerialManager.h"
#include <QDebug>
#include <cstring>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent)
{
    connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialManager::handleReadyRead);
    connect(&m_serialPort, &QSerialPort::errorOccurred, this, &SerialManager::handleError);
}

SerialManager::~SerialManager()
{
    closePort();
}

bool SerialManager::openPort(const QString &portName, qint32 baudRate)
{
    if (m_serialPort.isOpen()) {
        m_serialPort.close();
    }

    m_serialPort.setPortName(portName);
    m_serialPort.setBaudRate(baudRate);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort.open(QIODevice::ReadWrite)) {
        m_rxBuffer.clear();
        emit portStatusChanged(true, portName);
        return true;
    }

    emit errorOccurred(m_serialPort.errorString());
    emit portStatusChanged(false, portName);
    return false;
}

void SerialManager::closePort()
{
    if (m_serialPort.isOpen()) {
        m_serialPort.close();
        m_rxBuffer.clear();
        emit portStatusChanged(false, m_serialPort.portName());
    }
}

bool SerialManager::isOpen() const
{
    return m_serialPort.isOpen();
}

bool SerialManager::sendControlPacket(const ControlPacket::ControlPacket &packet)
{
    if (!m_serialPort.isOpen()) {
        emit errorOccurred("Cannot send command: Serial port is closed.");
        return false;
    }

    const char *data = reinterpret_cast<const char*>(&packet);
    qint64 bytesWritten = m_serialPort.write(data, sizeof(ControlPacket::ControlPacket));

    return (bytesWritten == sizeof(ControlPacket::ControlPacket));
}

void SerialManager::handleReadyRead()
{
    m_rxBuffer.append(m_serialPort.readAll());
    processRxBuffer();
}

void SerialManager::processRxBuffer()
{
    while (m_rxBuffer.size() >= 1) {
        const uint8_t sof = static_cast<uint8_t>(m_rxBuffer.at(0));

        // Audio Frame Header (SOF = 0xA6)
        if (sof == 0xA6) {
            if (m_rxBuffer.size() < static_cast<int>(sizeof(AudioFramePacket))) {
                return; // Czekamy na pełną ramkę
            }

            AudioFramePacket frame{};
            std::memcpy(&frame, m_rxBuffer.constData(), sizeof(AudioFramePacket));

            // Liczymy CRC z całego pakietu z wyłączeniem ostatnich 2 bajtów (pola crc16)
            const auto *rawBytes = reinterpret_cast<const uint8_t*>(m_rxBuffer.constData());
            constexpr std::size_t headerAndDataLen = sizeof(AudioFramePacket) - sizeof(uint16_t);
            const uint16_t computedCRC = Crc16Calculator::calculate(rawBytes, headerAndDataLen);

            if (computedCRC == frame.crc16) {
                emit audioFrameReceived(frame);
            } else {
                qDebug() << "AudioFramePacket CRC mismatch! Otrzymano:"
                         << frame.crc16 << "Obliczono:" << computedCRC;
            }

            m_rxBuffer.remove(0, sizeof(AudioFramePacket));
        }
        // Control Frame Header (SOF = 0xA5)
        else if (sof == 0xA5) {
            if (m_rxBuffer.size() < static_cast<int>(sizeof(ControlPacket::ControlPacket))) {
                return;
            }

            ControlPacket::ControlPacket packet{};
            std::memcpy(&packet, m_rxBuffer.constData(), sizeof(ControlPacket::ControlPacket));

            if (packet.isValid()) {
                emit controlPacketReceived(packet);
            } else {
                qDebug() << "ControlPacket CRC mismatch!";
            }

            m_rxBuffer.remove(0, sizeof(ControlPacket::ControlPacket));
        }
        // Błędny bajt synchronizacji -> odrzucamy 1 bajt i wyrównujemy
        else {
            m_rxBuffer.remove(0, 1);
        }
    }
}
void SerialManager::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        emit errorOccurred("Device disconnected unexpectedly.");
        closePort();
    }
}

// Ultra-fast CRC16 CCITT (Poly 0x1021, Init 0xFFFF)
uint16_t SerialManager::calculateCRC16(const uint8_t *data, std::size_t length) noexcept
{
    uint16_t crc = 0xFFFF;
    for (std::size_t i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}