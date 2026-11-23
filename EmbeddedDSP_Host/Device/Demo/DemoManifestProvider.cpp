#include "Device/Demo/DemoManifestProvider.h"
#include "Device/Demo/DemoDevice.h"
#include "Protocol/ProtocolCommon.h"

using namespace Host::Models;

namespace Host::Device::Demo {

DeviceManifest getDemoManifest() {
    DeviceManifest manifest;
    manifest.slotCount = DemoDevice::kSlotCount;

    // Ids from Protocol::EffectId (same as firmware effect classes).
    EffectSpec none;
    none.id = Protocol::EffectId::Empty;
    none.name = QStringLiteral("None");
    manifest.availableEffects[none.id] = none;

    EffectSpec delay;
    delay.id = Protocol::EffectId::Delay;
    delay.name = QStringLiteral("Delay");
    delay.params = {
        {QStringLiteral("Time"), 0.01f, 1.0f, 0.5f, QStringLiteral("s")},
        {QStringLiteral("Feedback"), 0.0f, 0.95f, 0.3f},
        {QStringLiteral("Mix"), 0.0f, 1.0f, 0.4f}
    };
    manifest.availableEffects[delay.id] = delay;

    EffectSpec overdrive;
    overdrive.id = Protocol::EffectId::Overdrive;
    overdrive.name = QStringLiteral("Overdrive");
    overdrive.params = {
        {QStringLiteral("Gain"), 1.0f, 20.0f, 5.0f},
        {QStringLiteral("Tone"), 0.1f, 1.0f, 0.5f}
    };
    manifest.availableEffects[overdrive.id] = overdrive;

    return manifest;
}

} // namespace Host::Device::Demo
