#include "StateManager.h"
#include "Logging.h"
#include <QStringList>
#include <QDebug>

namespace Host {

StateManager::StateManager(QObject* parent) : QObject(parent) {
    qCDebug(LOG_UI) << "Initializing StateManager...";
    m_workerThread = new QThread(this);

    // Create worker squad
    m_deviceInterface = new DeviceInterface();
    m_audioAnalyzer = new AudioAnalyzer();
    m_simulator = new AudioFrameSimulator();

    // Move squad to thread
    m_deviceInterface->moveToThread(m_workerThread);
    m_audioAnalyzer->moveToThread(m_workerThread);
    m_simulator->moveToThread(m_workerThread);

    // Cleanup squad on thread exit
    QObject::connect(m_workerThread, &QThread::finished, m_deviceInterface, &QObject::deleteLater);
    QObject::connect(m_workerThread, &QThread::finished, m_audioAnalyzer, &QObject::deleteLater);
    QObject::connect(m_workerThread, &QThread::finished, m_simulator, &QObject::deleteLater);

    // Internal squad coordination (happens in worker thread)
    QObject::connect(m_deviceInterface, &DeviceInterface::audioFrameReady, m_audioAnalyzer, &AudioAnalyzer::processFrame);
    QObject::connect(m_simulator, &AudioFrameSimulator::audioFrameReady, m_audioAnalyzer, &AudioAnalyzer::processFrame);

    // Signal-to-signal forwarding (Worker -> Manager/UI)
    QObject::connect(m_deviceInterface, &DeviceInterface::manifestReady, this, &StateManager::onDeviceManifestReady);
    QObject::connect(m_deviceInterface, &DeviceInterface::controlPacketReady, this, &StateManager::onControlPacketReady);
    QObject::connect(m_deviceInterface, &DeviceInterface::portStatusChanged, this, &StateManager::onConnectionChanged);
    QObject::connect(m_deviceInterface, &DeviceInterface::audioFrameReady, this, &StateManager::audioFrameReady);
    QObject::connect(m_simulator, &AudioFrameSimulator::audioFrameReady, this, &StateManager::audioFrameReady);
    QObject::connect(m_deviceInterface, &DeviceInterface::errorOccurred, this, &StateManager::errorOccurred);
    
    QObject::connect(m_audioAnalyzer, &AudioAnalyzer::spectrumReady, this, &StateManager::spectrumReady);
    QObject::connect(m_simulator, &AudioFrameSimulator::runningChanged, this, [this](bool running) {
        qCDebug(LOG_UI) << "Demo mode:" << (running ? "STARTED" : "STOPPED");
        emit demoRunningChanged(running);
    });

    m_workerThread->start();
}

StateManager::~StateManager() {
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

void StateManager::setParameter(uint8_t slotId, uint8_t paramId, float value) {
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

void StateManager::setEffectType(uint8_t slotId, uint8_t effectTypeId) {
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

void StateManager::setBypass(uint8_t slotId, bool bypassed) {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::BypassToggle;
    pkt.slotId = slotId;
    pkt.setValue(bypassed ? 1.0f : 0.0f);
    pkt.applyCRC();
    
    QMetaObject::invokeMethod(m_deviceInterface, [this, pkt]() {
        m_deviceInterface->sendControlPacket(pkt);
    }, Qt::QueuedConnection);
}

void StateManager::requestSync() {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::GetState;
    pkt.applyCRC();
    
    QMetaObject::invokeMethod(m_deviceInterface, [this, pkt]() {
        m_deviceInterface->sendControlPacket(pkt);
    }, Qt::QueuedConnection);
}

void StateManager::connect(const QString& target) {
    QMetaObject::invokeMethod(m_simulator, &AudioFrameSimulator::stop, Qt::QueuedConnection);
    
    QMetaObject::invokeMethod(m_deviceInterface, [this, target]() {
        m_deviceInterface->connect(target);
    }, Qt::QueuedConnection);
}

void StateManager::onConnectionChanged(bool connected, const QString& portName) {
    qCDebug(LOG_COMM) << "Connection status changed:" << (connected ? "CONNECTED" : "DISCONNECTED") << portName;
    if (connected) {
        requestSync();
    }
    emit connectionChanged(connected, portName);
}

void StateManager::disconnect() {
    QMetaObject::invokeMethod(m_deviceInterface, &DeviceInterface::disconnect, Qt::QueuedConnection);
}

void StateManager::startDemo() {
    disconnect(); // Close real connection if active
    QMetaObject::invokeMethod(m_simulator, &AudioFrameSimulator::start, Qt::QueuedConnection);
}

void StateManager::stopDemo() {
    QMetaObject::invokeMethod(m_simulator, &AudioFrameSimulator::stop, Qt::QueuedConnection);
}

void StateManager::onDeviceManifestReady(const DeviceManifest& manifest) {
    qCDebug(LOG_UI) << "Applying parsed manifest to UI state. Slots:" << manifest.slotCount;
    m_availableEffects = manifest.availableEffects;
    m_slotCount = manifest.slotCount;
    emit manifestReady();
}

void StateManager::onControlPacketReady(const Protocol::ControlPacket& pkt) {
    qCDebug(LOG_COMM) << "Control packet received. CMD:" << static_cast<int>(pkt.command) << "Slot:" << pkt.slotId;
    emit deviceStateUpdated(pkt);
}

} // namespace Host
