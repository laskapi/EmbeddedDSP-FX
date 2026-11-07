#include "Device/Demo/DemoDevice.h"
#include "Device/Demo/DemoConnectionConstants.h"
#include "Device/Demo/DemoAudioSimulator.h"
#include "Device/Demo/DemoManifestProvider.h"
#include "Protocol/ControlPacket.h"

namespace Host::Device {

DemoDevice::DemoDevice(QObject* parent)
    : IDevice(parent)
    , m_simulator(std::make_unique<Demo::DemoAudioSimulator>(this))
{
    connect(m_simulator.get(), &Demo::DemoAudioSimulator::audioFrameReady, this, &DemoDevice::audioFrameReady);
}

DemoDevice::~DemoDevice() {
    DemoDevice::close();
}

bool DemoDevice::open(const QString& connectionId) {
    if (m_isOpen) {
        return true;
    }

    m_isOpen = true;
    emit connectionStatusChanged(true, connectionId);
    return true;
}

void DemoDevice::close() {
    if (!m_isOpen) {
        return;
    }

    m_simulator->stop();
    m_isOpen = false;
    emit connectionStatusChanged(false, kDemoConnectionId);
}

bool DemoDevice::writeControlPacket(const Protocol::ControlPacket& pkt) {
    if (!m_isOpen) {
        return false;
    }

    // Mirror MCU beginSync: GetState → manifest, then Report*, then audio stream on.
    if (pkt.command == ::Protocol::Command::GetState) {
        emit manifestReady(Demo::getDemoManifest());

        for (uint8_t i = 0; i < 4; ++i) {
            ::Protocol::ControlPacket typePkt;
            typePkt.command = ::Protocol::Command::ReportEffectType;
            typePkt.slotId = i;
            typePkt.effectTypeId = i + 1;
            typePkt.setValue(0.0f);
            emit controlPacketReady(typePkt);

            ::Protocol::ControlPacket bypassPkt;
            bypassPkt.command = ::Protocol::Command::ReportBypass;
            bypassPkt.slotId = i;
            bypassPkt.setValue(0.0f); // not bypassed
            emit controlPacketReady(bypassPkt);

            for (uint8_t p = 0; p < 3; ++p) {
                ::Protocol::ControlPacket pPkt;
                pPkt.command = ::Protocol::Command::ReportParam;
                pPkt.slotId = i;
                pPkt.paramId = p;
                pPkt.setValue(0.5f);
                emit controlPacketReady(pPkt);
            }
        }

        ::Protocol::ControlPacket masterPkt;
        masterPkt.command = ::Protocol::Command::ReportMasterVolume;
        masterPkt.setValue(1.0f);
        emit controlPacketReady(masterPkt);

        m_simulator->start();
    }

    return true;
}

} // namespace Host::Device
