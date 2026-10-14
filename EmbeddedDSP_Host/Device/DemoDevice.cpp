#include "DemoDevice.h"
#include "ConnectionIds.h"
#include "DemoAudioSimulator.h"
#include "DemoManifestProvider.h"
#include <Protocol/ControlPacket.h>
#include <QTimer>

namespace Host::Device {

DemoDevice::DemoDevice(QObject* parent)
    : IDevice(parent)
    , m_simulator(std::make_unique<Demo::DemoAudioSimulator>(this))
{
    connect(m_simulator.get(), &Demo::DemoAudioSimulator::audioFrameReady, this, &DemoDevice::audioFrameReady);
}

DemoDevice::~DemoDevice() {
    DemoDevice::disconnectFromDevice();
}

bool DemoDevice::connectToDevice(const QString& target) {
    if (m_isOpen) {
        return true;
    }

    m_isOpen = true;
    emit connectionStatusChanged(true, target);

    QTimer::singleShot(100, this, [this]() {
        emit manifestReady(Demo::getDemoManifest());
        m_simulator->start();
    });

    return true;
}

void DemoDevice::disconnectFromDevice() {
    if (!m_isOpen) {
        return;
    }

    m_simulator->stop();
    m_isOpen = false;
    emit connectionStatusChanged(false, kDemoConnectionId);
}

bool DemoDevice::sendControlPacket(const Protocol::ControlPacket& pkt) {
    if (!m_isOpen) {
        return false;
    }

    if (pkt.command == ::Protocol::Command::GetState) {
        for (uint8_t i = 0; i < 4; ++i) {
            ::Protocol::ControlPacket statePkt;
            statePkt.command = ::Protocol::Command::SetEffectType;
            statePkt.slotId = i;
            statePkt.effectTypeId = i + 1;
            statePkt.setValue(0.0f);
            emit controlPacketReady(statePkt);

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
        emit controlPacketReady(pkt);
    }

    return true;
}

bool DemoDevice::isOpen() const {
    return m_isOpen;
}

} // namespace Host::Device
