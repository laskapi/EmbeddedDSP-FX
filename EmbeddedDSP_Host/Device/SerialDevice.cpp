#include "SerialDevice.h"
#include "DeviceManifestParser.h"
#include "Logging.h"
#include <Protocol/AudioFramePacket.h>
#include <Protocol/ControlPacket.h>
#include <cstring>

namespace Host::Device {

using Host::Models::parseDeviceManifest;

SerialDevice::SerialDevice(QObject* parent)
    : IDevice(parent)
    , m_serialPort(this)
{
    connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialDevice::handleReadyRead);
    connect(&m_serialPort, &QSerialPort::errorOccurred, this, &SerialDevice::handleError);
}

SerialDevice::~SerialDevice() {
    SerialDevice::close();
}

bool SerialDevice::open(const QString& connectionId) {
    if (connectionId.isEmpty()) {
        emit errorOccurred(tr("No connection selected."));
        emit connectionStatusChanged(false, QString());
        return false;
    }

    if (m_serialPort.isOpen()) {
        m_serialPort.close();
    }

    m_serialPort.setPortName(connectionId);
    m_serialPort.setBaudRate(Protocol::Hardware::DEFAULT_BAUD);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort.open(QIODevice::ReadWrite)) {
        m_rxBuffer.clear();
        m_manifestMode = false;
        emit connectionStatusChanged(true, connectionId);
        return true;
    }

    emit errorOccurred(m_serialPort.errorString());
    emit connectionStatusChanged(false, connectionId);
    return false;
}

void SerialDevice::close() {
    if (m_serialPort.isOpen()) {
        QString portName = m_serialPort.portName();
        m_serialPort.close();
        m_rxBuffer.clear();
        m_manifestMode = false;
        emit connectionStatusChanged(false, portName);
    }
}

bool SerialDevice::sendControlPacket(const Protocol::ControlPacket& pkt) {
    if (!m_serialPort.isOpen()) {
        return false;
    }
    return m_serialPort.write(reinterpret_cast<const char*>(&pkt), sizeof(pkt)) == sizeof(pkt);
}

void SerialDevice::handleReadyRead() {
    m_rxBuffer.append(m_serialPort.readAll());
    processRxBuffer();
}

void SerialDevice::processRxBuffer() {
    while (!m_rxBuffer.isEmpty()) {
        if (m_manifestMode) {
            int nullPos = m_rxBuffer.indexOf('\0');
            if (nullPos == -1) {
                return;
            }

            m_manifestBuffer.append(m_rxBuffer.left(nullPos));
            const QString manifest = QString::fromUtf8(m_manifestBuffer);

            const auto parsed = parseDeviceManifest(manifest);
            qCDebug(Host::Core::LOG_COMM) << "Manifest parsed. Slots:" << parsed.slotCount
                                          << "Effects:" << parsed.availableEffects.count();
            emit manifestReady(parsed);

            m_rxBuffer.remove(0, nullPos + 1);
            m_manifestBuffer.clear();
            m_manifestMode = false;
            continue;
        }

        const uint8_t sof = static_cast<uint8_t>(m_rxBuffer.at(0));

        if (sof == Protocol::SOF::Audio) {
            if (m_rxBuffer.size() < static_cast<int>(sizeof(Protocol::AudioFramePacket))) {
                return;
            }

            Protocol::AudioFramePacket frame;
            std::memcpy(&frame, m_rxBuffer.constData(), sizeof(frame));

            if (frame.isValid()) {
                emit audioFrameReady(frame);
            }
            m_rxBuffer.remove(0, sizeof(Protocol::AudioFramePacket));
        } else if (sof == Protocol::SOF::Control) {
            if (m_rxBuffer.size() < static_cast<int>(sizeof(Protocol::ControlPacket))) {
                return;
            }

            Protocol::ControlPacket pkt;
            std::memcpy(&pkt, m_rxBuffer.constData(), sizeof(pkt));

            if (pkt.isValid()) {
                if (pkt.command == Protocol::Command::ReportState &&
                    pkt.signalId == Protocol::ReservedParam::ManifestSignal) {
                    qCDebug(Host::Core::LOG_COMM) << "Entering manifest mode...";
                    m_manifestMode = true;
                    m_manifestBuffer.clear();
                } else {
                    emit controlPacketReady(pkt);
                }
            }
            m_rxBuffer.remove(0, sizeof(Protocol::ControlPacket));
        } else {
            m_rxBuffer.remove(0, 1);
        }
    }
}

void SerialDevice::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        emit errorOccurred(tr("Device disconnected."));
        close();
    }
}

} // namespace Host::Device
