#include "SerialManager.h"
#include <QDebug>
#include <QSerialPortInfo>
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

QString SerialManager::findDevicePort() {
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        if (info.hasVendorIdentifier() && info.vendorIdentifier() == 0x0483 &&
            info.hasProductIdentifier() && info.productIdentifier() == 0x5740) {
            return info.portName();
        }
    }
    return QString();
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
        m_manifestMode = false;
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
        m_manifestMode = false;
        emit portStatusChanged(false, m_serialPort.portName());
    }
}

bool SerialManager::isOpen() const
{
    return m_serialPort.isOpen();
}

bool SerialManager::sendControlPacket(const Protocol::ControlPacket &packet)
{
    if (!m_serialPort.isOpen()) return false;
    return m_serialPort.write(reinterpret_cast<const char*>(&packet), sizeof(packet)) == sizeof(packet);
}

void SerialManager::handleReadyRead()
{
    m_rxBuffer.append(m_serialPort.readAll());
    processRxBuffer();
}

void SerialManager::processRxBuffer()
{
    while (!m_rxBuffer.isEmpty()) {
        if (m_manifestMode) {
            int nullPos = m_rxBuffer.indexOf('\0');
            if (nullPos == -1) return;

            m_manifestBuffer.append(m_rxBuffer.left(nullPos));
            emit manifestReceived(QString::fromUtf8(m_manifestBuffer));
            
            m_rxBuffer.remove(0, nullPos + 1); 
            m_manifestBuffer.clear();
            m_manifestMode = false;
            continue; 
        }

        const uint8_t sof = static_cast<uint8_t>(m_rxBuffer.at(0));

        if (sof == Protocol::SOF::Audio) {
            if (m_rxBuffer.size() < (int)sizeof(Protocol::AudioFramePacket)) return;
            
            Protocol::AudioFramePacket frame;
            std::memcpy(&frame, m_rxBuffer.constData(), sizeof(frame));

            if (frame.isValid()) {
                emit audioFrameReceived(frame);
            }
            m_rxBuffer.remove(0, sizeof(Protocol::AudioFramePacket));
        }
        else if (sof == Protocol::SOF::Control) {
            if (m_rxBuffer.size() < (int)sizeof(Protocol::ControlPacket)) return;

            Protocol::ControlPacket packet;
            std::memcpy(&packet, m_rxBuffer.constData(), sizeof(packet));

            if (packet.isValid()) {
                if (packet.command == Protocol::Command::ReportState && 
                    packet.paramId == Protocol::ReservedParam::ManifestSignal) {
                    m_manifestMode = true;
                    m_manifestBuffer.clear();
                } else {
                    emit controlPacketReceived(packet);
                }
            }
            m_rxBuffer.remove(0, sizeof(Protocol::ControlPacket));
        }
        else {
            m_rxBuffer.remove(0, 1);
        }
    }
}

void SerialManager::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        emit errorOccurred(tr("Device disconnected."));
        closePort();
    }
}
