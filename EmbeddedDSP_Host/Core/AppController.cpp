#include "AppController.h"
#include "AudioFrameWorker.h"
#include "DeviceSession.h"
#include "IConnectionScanner.h"
#include "IDevice.h"
#include "IDeviceFactory.h"

#include <Protocol/ControlPacket.h>
#include <QThread>
#include <QTimer>
#include <QtGlobal>

using namespace Host::Processing;
using namespace Host::Device;
using namespace Host::Models;

namespace {

[[nodiscard]] uint8_t toProtocolByte(int value) {
    Q_ASSERT(value >= 0 && value <= 255);
    return static_cast<uint8_t>(value);
}

} // namespace

namespace Host::Core {

AppController::AppController(std::unique_ptr<IConnectionScanner> scanner,
                             std::unique_ptr<IDeviceFactory> factory,
                             std::unique_ptr<AudioFrameWorker> audioWorker,
                             QObject* parent)
    : QObject(parent)
    , m_scanner(std::move(scanner))
    , m_factory(std::move(factory))
{
    m_workerThread = new QThread(this);
    m_session = std::make_unique<DeviceSession>(m_workerThread, this);

    m_audioWorker = audioWorker.release();
    m_audioWorker->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::finished, m_audioWorker, &QObject::deleteLater);

    connect(m_audioWorker, &AudioFrameWorker::spectrumReady, this, &AppController::spectrumReady);
    connect(m_session.get(), &DeviceSession::deviceReady, this, &AppController::onDeviceReady);

    m_scanTimer = new QTimer(this);
    connect(m_scanTimer, &QTimer::timeout, this, &AppController::refreshAvailableConnections);
    m_scanTimer->start(2000);
    refreshAvailableConnections();

    m_workerThread->start();
}

AppController::~AppController() {
    if (m_session) {
        m_session->replace(std::unique_ptr<IDevice>{});
    }
    m_workerThread->quit();
    m_workerThread->wait();
}

void AppController::onDeviceReady(IDevice* device) {
    m_device = device;
    if (m_device) {
        setupDeviceSignals(m_device);
    }
}

void AppController::setupDeviceSignals(IDevice* device) {
    connect(device, &IDevice::manifestReady, this, &AppController::manifestReady);
    connect(device, &IDevice::audioFrameReady, m_audioWorker, &AudioFrameWorker::processFrame);
    connect(device, &IDevice::controlPacketReady, this, &AppController::onControlPacketReady);
    connect(device, &IDevice::connectionStatusChanged, this, &AppController::onConnectionStatusChanged);
    connect(device, &IDevice::errorOccurred, this, &AppController::errorOccurred);
}

void AppController::connectToDevice(const QString& target) {
    if (!m_factory || !m_session) {
        return;
    }

    if (m_isDeviceConnected) {
        disconnectFromDevice();
    }

    auto device = m_factory->create(target);
    m_session->replace(std::move(device));

    if (!m_device) {
        return;
    }

    QMetaObject::invokeMethod(m_device, &IDevice::connectToDevice, Qt::QueuedConnection, target);
}

void AppController::disconnectFromDevice() {
    if (!m_device) {
        return;
    }

    QMetaObject::invokeMethod(m_device, &IDevice::disconnectFromDevice, Qt::QueuedConnection);
}

void AppController::refreshAvailableConnections() {
    if (!m_scanner || m_isDeviceConnected) {
        return;
    }
    emit connectionsUpdated(m_scanner->scan());
}

void AppController::dispatchControlPacket(::Protocol::ControlPacket pkt) {
    if (!m_device) {
        return;
    }

    pkt.applyCRC();
    QMetaObject::invokeMethod(m_device, &IDevice::sendControlPacket, Qt::QueuedConnection, pkt);
}

void AppController::requestSync() {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::GetState;
    dispatchControlPacket(pkt);
}

void AppController::setParameter(int slotIndex, int paramIndex, float value) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::SetParam;
    pkt.slotId = toProtocolByte(slotIndex);
    pkt.paramId = toProtocolByte(paramIndex);
    pkt.setValue(value);
    dispatchControlPacket(pkt);
}

void AppController::setEffectType(int slotIndex, int effectTypeId) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::SetEffectType;
    pkt.slotId = toProtocolByte(slotIndex);
    pkt.effectTypeId = toProtocolByte(effectTypeId);
    pkt.setValue(0.0f);
    dispatchControlPacket(pkt);
}

void AppController::setBypass(int slotIndex, bool bypass) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::BypassToggle;
    pkt.slotId = toProtocolByte(slotIndex);
    pkt.setValue(bypass ? 1.0f : 0.0f);
    dispatchControlPacket(pkt);
}

void AppController::onControlPacketReady(const Protocol::ControlPacket& pkt) {
    using ::Protocol::Command;

    switch (pkt.command) {
    case Command::SetParam:
        emit slotParameterUpdated(static_cast<int>(pkt.slotId),
                                  static_cast<int>(pkt.paramId),
                                  pkt.getValue());
        break;
    case Command::SetEffectType:
        emit slotEffectTypeUpdated(static_cast<int>(pkt.slotId),
                                   static_cast<int>(pkt.effectTypeId));
        break;
    case Command::BypassToggle:
        emit slotBypassUpdated(static_cast<int>(pkt.slotId), pkt.getValue() > 0.5f);
        break;
    default:
        break;
    }
}

void AppController::onConnectionStatusChanged(bool connected, const QString& portName) {
    m_isDeviceConnected = connected;
    emit connectionChanged(connected, portName);
    if (!connected) {
        refreshAvailableConnections();
    }
}

} // namespace Host::Core
