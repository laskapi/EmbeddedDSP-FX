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
    // Match DelayEffect::kMaxTimeSec (SharedBuffer 64 KiB / float - 2) @ SAMPLE_RATE_HZ.
    constexpr float kDelayMaxTimeSec =
        static_cast<float>((64 * 1024) / 4 - 2) / Protocol::SAMPLE_RATE_HZ;
    delay.params = {
        {QStringLiteral("Time"), 0.01f, kDelayMaxTimeSec, 0.25f, QStringLiteral("s")},
        {QStringLiteral("Feedback"), 0.0f, 0.95f, 0.3f},
        {QStringLiteral("Mix"), 0.0f, 1.0f, 0.4f}
    };
    manifest.availableEffects[delay.id] = delay;

    EffectSpec overdrive;
    overdrive.id = Protocol::EffectId::Overdrive;
    overdrive.name = QStringLiteral("Overdrive");
    overdrive.params = {
        {QStringLiteral("Gain"), 1.0f, 20.0f, 5.0f},
        {QStringLiteral("Tone"), 0.1f, 1.0f, 0.5f},
        {QStringLiteral("Level"), 0.0f, 1.0f, 0.7f}
    };
    manifest.availableEffects[overdrive.id] = overdrive;

    EffectSpec tremolo;
    tremolo.id = Protocol::EffectId::Tremolo;
    tremolo.name = QStringLiteral("Tremolo");
    tremolo.params = {
        {QStringLiteral("Rate"), 0.5f, 12.0f, 4.0f, QStringLiteral("Hz")},
        {QStringLiteral("Depth"), 0.0f, 1.0f, 0.5f},
        {QStringLiteral("Stereo"), 0.0f, 1.0f, 0.0f}
    };
    manifest.availableEffects[tremolo.id] = tremolo;

    EffectSpec bitcrusher;
    bitcrusher.id = Protocol::EffectId::Bitcrusher;
    bitcrusher.name = QStringLiteral("Bitcrusher");
    bitcrusher.params = {
        {QStringLiteral("Bits"), 4.0f, 16.0f, 8.0f},
        {QStringLiteral("Downsample"), 1.0f, 16.0f, 4.0f},
        {QStringLiteral("Tone"), 0.1f, 1.0f, 1.0f},
        {QStringLiteral("Mix"), 0.0f, 1.0f, 1.0f}
    };
    manifest.availableEffects[bitcrusher.id] = bitcrusher;

    return manifest;
}

} // namespace Host::Device::Demo
