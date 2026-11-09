#include "Device/Demo/DemoDevice.h"
#include "Device/Demo/DemoConnectionConstants.h"
#include "Device/Demo/DemoAudioSimulator.h"
#include "Device/Demo/DemoManifestProvider.h"
#include "Models/DeviceModel.h"
#include "Protocol/ControlPacket.h"

#include <utility>

namespace Host::Device {

namespace {

[[nodiscard]] const Host::Models::EffectSpec* findSpec(int effectTypeId) {
    static const Host::Models::DeviceManifest kManifest = Demo::getDemoManifest();
    const auto it = kManifest.availableEffects.constFind(effectTypeId);
    if (it == kManifest.availableEffects.cend()) {
        return nullptr;
    }
    return &(*it);
}

} // namespace

DemoDevice::DemoDevice(QObject* parent)
    : IDevice(parent)
    , m_simulator(std::make_unique<Demo::DemoAudioSimulator>(this))
{
    connect(m_simulator.get(), &Demo::DemoAudioSimulator::audioFrameReady, this, &DemoDevice::audioFrameReady);
    resetSlotsToEmpty();
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

void DemoDevice::resetSlotsToEmpty() {
    for (auto& slot : m_slots) {
        slot = SlotState{};
    }
    m_masterGain = kDefaultMasterGain;
}

void DemoDevice::applyEffectDefaults(int slotIndex, int effectTypeId) {
    auto& slot = m_slots[static_cast<std::size_t>(slotIndex)];
    slot.effectTypeId = effectTypeId;
    slot.bypassed = false;
    slot.params.fill(0.0f);

    if (const auto* spec = findSpec(effectTypeId)) {
        for (int i = 0; i < spec->params.size() && i < kMaxParams; ++i) {
            slot.params[static_cast<std::size_t>(i)] = spec->params[i].defaultValue;
        }
    }
}

void DemoDevice::emitSlotReports(int slotIndex) {
    const auto& slot = m_slots[static_cast<std::size_t>(slotIndex)];

    ::Protocol::ControlPacket typePkt;
    typePkt.command = ::Protocol::Command::ReportEffectType;
    typePkt.slotId = static_cast<uint8_t>(slotIndex);
    typePkt.effectTypeId = static_cast<uint8_t>(slot.effectTypeId);
    typePkt.setValue(0.0f);
    emit controlPacketReady(typePkt);

    emitBypassReport(slotIndex);

    if (const auto* spec = findSpec(slot.effectTypeId)) {
        for (int p = 0; p < spec->params.size() && p < kMaxParams; ++p) {
            ::Protocol::ControlPacket pPkt;
            pPkt.command = ::Protocol::Command::ReportParam;
            pPkt.slotId = static_cast<uint8_t>(slotIndex);
            pPkt.paramId = static_cast<uint8_t>(p);
            pPkt.setValue(slot.params[static_cast<std::size_t>(p)]);
            emit controlPacketReady(pPkt);
        }
    }
}

void DemoDevice::emitBypassReport(int slotIndex) {
    const auto& slot = m_slots[static_cast<std::size_t>(slotIndex)];
    ::Protocol::ControlPacket bypassPkt;
    bypassPkt.command = ::Protocol::Command::ReportBypass;
    bypassPkt.slotId = static_cast<uint8_t>(slotIndex);
    bypassPkt.setValue(slot.bypassed ? 1.0f : 0.0f);
    emit controlPacketReady(bypassPkt);
}

void DemoDevice::emitMasterVolumeReport() {
    ::Protocol::ControlPacket masterPkt;
    masterPkt.command = ::Protocol::Command::ReportMasterVolume;
    masterPkt.setValue(m_masterGain);
    emit controlPacketReady(masterPkt);
}

bool DemoDevice::writeControlPacket(const Protocol::ControlPacket& pkt) {
    if (!m_isOpen) {
        return false;
    }

    using ::Protocol::Command;

    switch (pkt.command) {
    case Command::GetState: {
        const auto manifest = Demo::getDemoManifest();
        emit manifestReady(manifest);

        for (int i = 0; i < kSlotCount; ++i) {
            emitSlotReports(i);
        }
        emitMasterVolumeReport();
        m_simulator->start();
        break;
    }
    case Command::SetEffectType: {
        const int slot = pkt.slotId;
        if (slot < 0 || slot >= kSlotCount) {
            break;
        }
        applyEffectDefaults(slot, pkt.effectTypeId);
        emitSlotReports(slot);
        break;
    }
    case Command::SetParam: {
        const int slot = pkt.slotId;
        const int param = pkt.paramId;
        if (slot < 0 || slot >= kSlotCount || param < 0 || param >= kMaxParams) {
            break;
        }
        // Option A: no ReportParam echo (matches MCU).
        m_slots[static_cast<std::size_t>(slot)].params[static_cast<std::size_t>(param)] = pkt.value();
        break;
    }
    case Command::BypassToggle: {
        const int slot = pkt.slotId;
        if (slot < 0 || slot >= kSlotCount) {
            break;
        }
        m_slots[static_cast<std::size_t>(slot)].bypassed = pkt.value() > 0.5f;
        emitBypassReport(slot);
        break;
    }
    case Command::ClearSlot: {
        const int slot = pkt.slotId;
        if (slot < 0 || slot >= kSlotCount) {
            break;
        }
        applyEffectDefaults(slot, 0);
        emitSlotReports(slot);
        break;
    }
    case Command::SwapSlots: {
        const int a = pkt.slotId;
        const int b = pkt.targetSlotId;
        if (a < 0 || a >= kSlotCount || b < 0 || b >= kSlotCount) {
            break;
        }
        std::swap(m_slots[static_cast<std::size_t>(a)], m_slots[static_cast<std::size_t>(b)]);
        emitSlotReports(a);
        emitSlotReports(b);
        break;
    }
    case Command::SetMasterVolume:
        m_masterGain = pkt.value();
        emitMasterVolumeReport();
        break;
    default:
        break;
    }

    return true;
}

} // namespace Host::Device
