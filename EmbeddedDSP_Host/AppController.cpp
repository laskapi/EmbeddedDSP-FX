#include "AppController.h"
#include "Logging.h"
#include "DeviceInterface.h"
#include "AudioFrameWorker.h"
#include "AudioFrameSimulator.h"
#include <Protocol/ControlPacket.h>
#include <Protocol/AudioFramePacket.h>
#include <QThread>
#include <QDebug>

namespace Host {

AppController::AppController() : QObject(nullptr) {
    qCDebug(LOG_UI) << "Initializing AppController Hub...";
    m_workerThread = new QThread(this);

    // Create worker squad
    m_deviceInterface = new DeviceInterface();
    m_audioWorker = new AudioFrameWorker();
    m_simulator = new AudioFrameSimulator();

    // Move squad to thread
    m_deviceInterface->moveToThread(m_workerThread);
    m_audioWorker->moveToThread(m_workerThread);
    m_simulator->moveToThread(m_workerThread);

    // Cleanup squad on thread exit
    QObject::connect(m_workerThread, &QThread::finished, m_deviceInterface, &QObject::deleteLater);
    QObject::connect(m_workerThread, &QThread::finished, m_audioWorker, &QObject::deleteLater);
    QObject::connect(m_workerThread, &QThread::finished, m_simulator, &QObject::deleteLater);

    // Internal squad coordination (happens in worker thread)
    QObject::connect(m_deviceInterface, &DeviceInterface::audioFrameReady, m_audioWorker, &AudioFrameWorker::processFrame);
    QObject::connect(m_simulator, &AudioFrameSimulator::audioFrameReady, m_audioWorker, &AudioFrameWorker::processFrame);

    // Signal-to-signal forwarding (Worker -> Controller -> UI)
    QObject::connect(m_deviceInterface, &DeviceInterface::manifestReady, this, &AppController::manifestReady);
    QObject::connect(m_deviceInterface, &DeviceInterface::controlPacketReady, this, &AppController::deviceStateUpdated);
    QObject::connect(m_deviceInterface, &DeviceInterface::portStatusChanged, this, &AppController::onConnectionChanged);
    QObject::connect(m_deviceInterface, &DeviceInterface::audioFrameReady, this, &AppController::audioFrameReady);
    QObject::connect(m_simulator, &AudioFrameSimulator::audioFrameReady, this, &AppController::audioFrameReady);
    QObject::connect(m_deviceInterface, &DeviceInterface::errorOccurred, this, &AppController::errorOccurred);
    
    QObject::connect(m_audioWorker, &AudioFrameWorker::spectrumReady, this, &AppController::spectrumReady);
    QObject::connect(m_simulator, &AudioFrameSimulator::runningChanged, this, [this](bool running) {
        qCDebug(LOG_UI) << "Demo mode:" << (running ? "STARTED" : "STOPPED");
        emit demoRunningChanged(running);
    });

    m_workerThread->start();
}

AppController::~AppController() {
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

void AppController::setParameter(uint8_t slotId, uint8_t paramId, float value) {
    qCDebug(LOG_COMM) << "SetParam requested. Slot:" << slotId << "Param:" << paramId << "Val:" << value;
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::SetParam;
    pkt.slotId = slotId;
    pkt.paramId = paramId;
    pkt.setValue(value);
    pkt.applyCRC();
    
    QMetaObject::invokeMethod(m_deviceInterface, [this, pkt]() {
        m_deviceInterface->sendControlPacket(pkt);
    }, Qt::QueuedConnection);
}

void AppController::setEffectType(uint8_t slotId, uint8_t effectTypeId) {
    qCDebug(LOG_COMM) << "SetEffectType requested. Slot:" << slotId << "Type:" << effectTypeId;
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::SetEffectType;
    pkt.slotId = slotId;
    pkt.effectTypeId = effectTypeId;
    pkt.applyCRC();
    
    QMetaObject::invokeMethod(m_deviceInterface, [this, pkt]() {
        m_deviceInterface->sendControlPacket(pkt);
    }, Qt::QueuedConnection);
}

void AppController::setBypass(uint8_t slotId, bool bypassed) {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::BypassToggle;
    pkt.slotId = slotId;
    pkt.setValue(bypassed ? 1.0f : 0.0f);
    pkt.applyCRC();
    
    QMetaObject::invokeMethod(m_deviceInterface, [this, pkt]() {
        m_deviceInterface->sendControlPacket(pkt);
    }, Qt::QueuedConnection);
}

void AppController::requestSync() {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::GetState;
    pkt.applyCRC();
    
    QMetaObject::invokeMethod(m_deviceInterface, [this, pkt]() {
        m_deviceInterface->sendControlPacket(pkt);
    }, Qt::QueuedConnection);
}

void AppController::connect(const QString& target) {
    QMetaObject::invokeMethod(m_simulator, &AudioFrameSimulator::stop, Qt::QueuedConnection);
    
    QMetaObject::invokeMethod(m_deviceInterface, [this, target]() {
        m_deviceInterface->connect(target);
    }, Qt::QueuedConnection);
}

void AppController::onConnectionChanged(bool connected, const QString& portName) {
    qCDebug(LOG_COMM) << "Connection status changed:" << (connected ? "CONNECTED" : "DISCONNECTED") << portName;
    if (connected) {
        requestSync();
    }
    emit connectionChanged(connected, portName);
}

void AppController::disconnect() {
    QMetaObject::invokeMethod(m_deviceInterface, &DeviceInterface::disconnect, Qt::QueuedConnection);
}

void AppController::startDemo() {
    disconnect(); 
    QMetaObject::invokeMethod(m_simulator, &AudioFrameSimulator::start, Qt::QueuedConnection);
}

void AppController::stopDemo() {
    QMetaObject::invokeMethod(m_simulator, &AudioFrameSimulator::stop, Qt::QueuedConnection);
}

} // namespace Host
