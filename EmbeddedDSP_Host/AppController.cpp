#include "AppController.h"
#include <QDebug>
#include <QThread>

#include "AudioFrameWorker.h"
#include "DeviceInterface.h"

#include <Protocol/AudioFramePacket.h>
#include <Protocol/ControlPacket.h>

#ifdef HOST_SIMULATOR_ENABLED
#include "AudioFrameSimulator.h"
#include "DemoManifestProvider.h"
#endif

namespace Host {

AppController::AppController() : QObject(nullptr) {
    m_workerThread = new QThread(this);
    m_audioWorker = new AudioFrameWorker();
    m_audioWorker->moveToThread(m_workerThread);
    
    QObject::connect(m_workerThread, &QThread::finished, m_audioWorker, &QObject::deleteLater);
    QObject::connect(m_audioWorker, &AudioFrameWorker::spectrumReady, this, &AppController::spectrumReady);

#ifdef HOST_SIMULATOR_ENABLED
    m_simulator = new AudioFrameSimulator();
    m_simulator->moveToThread(m_workerThread);
    
    QObject::connect(m_workerThread, &QThread::finished, m_simulator, &QObject::deleteLater);
    QObject::connect(m_simulator, &AudioFrameSimulator::audioFrameReady, m_audioWorker, &AudioFrameWorker::processFrame);
    QObject::connect(m_simulator, &AudioFrameSimulator::audioFrameReady, this, &AppController::audioFrameReady);
    QObject::connect(m_simulator, &AudioFrameSimulator::runningChanged, this, &AppController::demoRunningChanged);
#else
    m_deviceInterface = new DeviceInterface();
    m_deviceInterface->moveToThread(m_workerThread);

    QObject::connect(m_workerThread, &QThread::finished, m_deviceInterface, &QObject::deleteLater);
    QObject::connect(m_deviceInterface, &DeviceInterface::audioFrameReady, m_audioWorker, &AudioFrameWorker::processFrame);
    QObject::connect(m_deviceInterface, &DeviceInterface::audioFrameReady, this, &AppController::audioFrameReady);
    QObject::connect(m_deviceInterface, &DeviceInterface::manifestReady, this, &AppController::manifestReady);
    QObject::connect(m_deviceInterface, &DeviceInterface::controlPacketReady, this, &AppController::deviceStateUpdated);
    QObject::connect(m_deviceInterface, &DeviceInterface::portStatusChanged, this, &AppController::onConnectionChanged);
    QObject::connect(m_deviceInterface, &DeviceInterface::errorOccurred, this, &AppController::errorOccurred);
#endif

    m_workerThread->start();
}

AppController::~AppController() {
    m_workerThread->quit();
    m_workerThread->wait();
}

void AppController::connect(const QString& target) {
#ifdef HOST_SIMULATOR_ENABLED
    Q_UNUSED(target);
    injectDemoData();
    emit connectionChanged(true, QStringLiteral("Simulator"));
#else
    QMetaObject::invokeMethod(m_deviceInterface, [this, target]() {
        m_deviceInterface->connect(target);
    }, Qt::QueuedConnection);
#endif
}

void AppController::disconnect() {
#ifdef HOST_SIMULATOR_ENABLED
    if (m_simulator) m_simulator->stop();
    emit connectionChanged(false, QString());
#else
    QMetaObject::invokeMethod(m_deviceInterface, &DeviceInterface::disconnect, Qt::QueuedConnection);
#endif
}

#ifdef HOST_SIMULATOR_ENABLED
void AppController::startDemo() {
    if (m_simulator)
        QMetaObject::invokeMethod(m_simulator, &AudioFrameSimulator::start, Qt::QueuedConnection);
}

void AppController::stopDemo() {
    if (m_simulator)
        QMetaObject::invokeMethod(m_simulator, &AudioFrameSimulator::stop, Qt::QueuedConnection);
}
#endif

void AppController::requestSync() {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::GetState;
    dispatchControlPacket(pkt);
}

void AppController::dispatchControlPacket(::Protocol::ControlPacket pkt) {
#ifdef HOST_SIMULATOR_ENABLED
    if (pkt.command == ::Protocol::Command::GetState) {
        for (uint8_t i = 0; i < 4; ++i) {
            ::Protocol::ControlPacket statePkt;
            statePkt.command = ::Protocol::Command::SetEffectType;
            statePkt.slotId = i;
            statePkt.effectTypeId = i + 1;
            statePkt.setValue(0.0f);
            emit deviceStateUpdated(statePkt);
            
            for (uint8_t p = 0; p < 3; ++p) {
                ::Protocol::ControlPacket pPkt;
                pPkt.command = ::Protocol::Command::SetParam;
                pPkt.slotId = i;
                pPkt.paramId = p;
                pPkt.setValue(0.5f);
                emit deviceStateUpdated(pPkt);
            }
        }
    } else {
        emit deviceStateUpdated(pkt);
    }
#else
    pkt.applyCRC();
    if (m_deviceInterface) {
        QMetaObject::invokeMethod(m_deviceInterface, [this, pkt]() {
            m_deviceInterface->sendControlPacket(pkt);
        }, Qt::QueuedConnection);
    }
#endif
}

void AppController::setParameter(uint8_t slotId, uint8_t paramId, float value) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::SetParam;
    pkt.slotId = slotId;
    pkt.paramId = paramId;
    pkt.setValue(value);
    dispatchControlPacket(pkt);
}

void AppController::setEffectType(uint8_t slotId, uint8_t effectTypeId) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::SetEffectType;
    pkt.slotId = slotId;
    pkt.effectTypeId = effectTypeId;
    pkt.setValue(0.0f);
    dispatchControlPacket(pkt);
}

void AppController::setBypass(uint8_t slotId, bool bypass) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::BypassToggle;
    pkt.slotId = slotId;
    pkt.setValue(bypass ? 1.0f : 0.0f);
    dispatchControlPacket(pkt);
}

void AppController::injectDemoData() {
#ifdef HOST_SIMULATOR_ENABLED
    emit manifestReady(DemoManifestProvider::get());
#endif
}

void AppController::onConnectionChanged(bool connected, const QString& portName) {
    emit connectionChanged(connected, portName);
}

} // namespace Host
