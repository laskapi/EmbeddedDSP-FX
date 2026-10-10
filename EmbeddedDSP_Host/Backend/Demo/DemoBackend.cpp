#include "DemoBackend.h"
#include "DemoAudioSimulator.h"
#include "DemoManifestProvider.h"
#include <Protocol/ControlPacket.h>
#include <Protocol/AudioFramePacket.h>
#include <QTimer>

namespace {

const QString kDemoConnectionId = QStringLiteral("Simulator");
const QString kDemoConnectionDescription = QStringLiteral("Internal DSP Simulator");

} // namespace

namespace Host::Backend {

using Host::Models::ConnectionInfo;

DemoBackend::DemoBackend(QObject *parent)
    : IDeviceBackend(parent)
    , m_simulator(std::make_unique<Demo::DemoAudioSimulator>(this))
{
    connect(m_simulator.get(), &Demo::DemoAudioSimulator::audioFrameReady, this, &DemoBackend::audioFrameReady);
}

DemoBackend::~DemoBackend() {
    disconnectFromDevice();
}

bool DemoBackend::connectToDevice(const QString &target) {
    if (m_isOpen) return true;

    m_isOpen = true;
    emit connectionStatusChanged(true, target);

    QTimer::singleShot(100, this, [this]() {
        emit manifestReady(Demo::getDemoManifest());
        m_simulator->start();
    });

    return true;
}

void DemoBackend::disconnectFromDevice() {
    if (!m_isOpen) return;

    m_simulator->stop();
    m_isOpen = false;
    emit connectionStatusChanged(false, kDemoConnectionId);
}

bool DemoBackend::sendControlPacket(const Protocol::ControlPacket &pkt) {
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

bool DemoBackend::isOpen() const {
    return m_isOpen;
}

std::vector<ConnectionInfo> DemoBackend::availableConnections() const {
    return {{kDemoConnectionId, kDemoConnectionDescription}};
}

} // namespace Host::Backend
