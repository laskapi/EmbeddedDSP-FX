#include "SimulatorBackend.h"
#include "AudioFrameSimulator.h"
#include "DemoManifestProvider.h"
#include <Protocol/ControlPacket.h>
#include <Protocol/AudioFramePacket.h>
#include <QTimer>

namespace Host {

SimulatorBackend::SimulatorBackend(QObject *parent)
    : IDeviceBackend(parent)
    , m_simulator(std::make_unique<AudioFrameSimulator>(this))
{
    connect(m_simulator.get(), &AudioFrameSimulator::audioFrameReady, this, &SimulatorBackend::audioFrameReady);
}

SimulatorBackend::~SimulatorBackend() {
    disconnect();
}

bool SimulatorBackend::connect(const QString &target) {
    if (m_isOpen) return true;
    
    m_isOpen = true;
    emit connectionStatusChanged(true, target);
    
    QTimer::singleShot(100, this, [this]() {
        emit manifestReady(DemoManifestProvider::get());
        m_simulator->start();
    });
    
    return true;
}

void SimulatorBackend::disconnect() {
    if (!m_isOpen) return;
    
    m_simulator->stop();
    m_isOpen = false;
    emit connectionStatusChanged(false, QStringLiteral("Simulator"));
}

bool SimulatorBackend::sendControlPacket(const Protocol::ControlPacket &pkt) {
    if (!m_isOpen) return false;

    if (pkt.command == ::Protocol::Command::GetState) {
        // Report initial state for demo slots
        for (uint8_t i = 0; i < 4; ++i) {
            ::Protocol::ControlPacket statePkt;
            statePkt.command = ::Protocol::Command::SetEffectType;
            statePkt.slotId = i;
            statePkt.effectTypeId = i + 1;
            statePkt.setValue(0.0f);
            emit controlPacketReady(statePkt);
            
            // Default params
            for (uint8_t p = 0; p < 3; ++p) {
                ::Protocol::ControlPacket pPkt;
                pPkt.command = ::Protocol::Command::SetParam;
                pPkt.slotId = i;
                pPkt.paramId = p;
                pPkt.setValue(0.5f);
                emit controlPacketReady(pPkt);
            }
        }
    } else {
        // Mirror packet back to UI for all other commands (SetParam, SetEffectType, Bypass)
        emit controlPacketReady(pkt);
    }
    
    return true;
}

bool SimulatorBackend::isOpen() const {
    return m_isOpen;
}

std::vector<ConnectionInfo> SimulatorBackend::enumerateConnections() const {
    return {{QStringLiteral("Simulator"), QStringLiteral("Internal DSP Simulator")}};
}

} // namespace Host
