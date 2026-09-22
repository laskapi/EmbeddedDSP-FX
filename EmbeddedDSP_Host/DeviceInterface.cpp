#include "DeviceInterface.h"
#include "Logging.h"
#include <QDebug>
#include <QSerialPortInfo>
#include <QStringList>
#include <cstring>

namespace Host {

DeviceInterface::DeviceInterface(QObject *parent)
    : QObject(parent), m_serialPort(this)
{
    QObject::connect(&m_serialPort, &QSerialPort::readyRead, this, &DeviceInterface::handleReadyRead);
    QObject::connect(&m_serialPort, &QSerialPort::errorOccurred, this, &DeviceInterface::handleError);
}

DeviceInterface::~DeviceInterface()
{
    disconnect();
}

QString DeviceInterface::findDevicePort() {
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        if (info.hasVendorIdentifier() && info.vendorIdentifier() == Protocol::Hardware::USB_VID &&
            info.hasProductIdentifier() && info.productIdentifier() == Protocol::Hardware::USB_PID) {
            return info.portName();
        }
    }
    return QString();
}

bool DeviceInterface::connect(const QString &target)
{
    QString finalTarget = target;
    if (finalTarget.isEmpty()) {
        finalTarget = findDevicePort();
    }

    if (finalTarget.isEmpty()) {
        emit errorOccurred(tr("No compatible device found."));
        emit portStatusChanged(false, QString());
        return false;
    }

    if (m_serialPort.isOpen()) {
        m_serialPort.close();
    }

    m_serialPort.setPortName(finalTarget);
    m_serialPort.setBaudRate(Protocol::Hardware::DEFAULT_BAUD);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort.open(QIODevice::ReadWrite)) {
        m_rxBuffer.clear();
        m_manifestMode = false;
        emit portStatusChanged(true, finalTarget);
        return true;
    }

    emit errorOccurred(m_serialPort.errorString());
    emit portStatusChanged(false, finalTarget);
    return false;
}

void DeviceInterface::disconnect()
{
    if (m_serialPort.isOpen()) {
        m_serialPort.close();
        m_rxBuffer.clear();
        m_manifestMode = false;
        emit portStatusChanged(false, m_serialPort.portName());
    }
}

bool DeviceInterface::isOpen() const
{
    return m_serialPort.isOpen();
}

bool DeviceInterface::sendControlPacket(const Protocol::ControlPacket &pkt)
{
    if (!m_serialPort.isOpen()) return false;
    return m_serialPort.write(reinterpret_cast<const char*>(&pkt), sizeof(pkt)) == sizeof(pkt);
}

void DeviceInterface::handleReadyRead()
{
    m_rxBuffer.append(m_serialPort.readAll());
    processRxBuffer();
}

void DeviceInterface::processRxBuffer()
{
    while (!m_rxBuffer.isEmpty()) {
        if (m_manifestMode) {
            int nullPos = m_rxBuffer.indexOf('\0');
            if (nullPos == -1) return;

            m_manifestBuffer.append(m_rxBuffer.left(nullPos));
            const QString manifest = QString::fromUtf8(m_manifestBuffer);
            
            qCDebug(LOG_COMM) << "Manifest received, length:" << manifest.length();
            emit manifestReady(parseManifest(manifest));
            
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
                emit audioFrameReady(frame);
            }
            m_rxBuffer.remove(0, sizeof(Protocol::AudioFramePacket));
        }
        else if (sof == Protocol::SOF::Control) {
            if (m_rxBuffer.size() < (int)sizeof(Protocol::ControlPacket)) return;

            Protocol::ControlPacket pkt;
            std::memcpy(&pkt, m_rxBuffer.constData(), sizeof(pkt));

            if (pkt.isValid()) {
                if (pkt.command == Protocol::Command::ReportState && 
                    pkt.signalId == Protocol::ReservedParam::ManifestSignal) {
                    qCDebug(LOG_COMM) << "Entering manifest mode...";
                    m_manifestMode = true;
                    m_manifestBuffer.clear();
                } else {
                    emit controlPacketReady(pkt);
                }
            }
            m_rxBuffer.remove(0, sizeof(Protocol::ControlPacket));
        }
        else {
            m_rxBuffer.remove(0, 1);
        }
    }
}

Host::DeviceManifest DeviceInterface::parseManifest(const QString &manifest) {
    Host::DeviceManifest result;
    const QStringList effectLines = manifest.split('\n', Qt::SkipEmptyParts);

    for (const QString &effectLine : effectLines) {
        QStringList segments = effectLine.split(';', Qt::SkipEmptyParts);
        if (segments.isEmpty()) continue;

        QStringList header = segments[0].split(':');
        if (header.isEmpty()) continue;

        const QString tag = header[0];

        if (tag == QLatin1String("C")) {
            if (header.size() >= 3 && header[1] == QLatin1String("SLOTS")) {
                result.slotCount = header[2].toInt();
            }
        } else if (tag == QLatin1String("E")) {
            if (header.size() < 3) continue;

            Host::EffectSpec spec;
            spec.id = static_cast<uint8_t>(header[1].toInt());
            spec.name = header[2];

            auto params = segments.sliced(1);
            for (const QString &param : std::as_const(params)) {
                QStringList p = param.split(':');
                if (p.size() < 5 || p[0] != QLatin1String("P")) continue;
                
                spec.params.append({p[1], p[2].toFloat(), p[3].toFloat(), p[4].toFloat()});
            }
            result.availableEffects[spec.id] = spec;
        }
    }

    if (result.slotCount <= 0) result.slotCount = 4;
    
    qCDebug(LOG_COMM) << "Manifest parsed. Slots:" << result.slotCount 
                     << "Effects:" << result.availableEffects.count();
    
    return result;
}

void DeviceInterface::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        emit errorOccurred(tr("Device disconnected."));
        disconnect();
    }
}

} // namespace Host


