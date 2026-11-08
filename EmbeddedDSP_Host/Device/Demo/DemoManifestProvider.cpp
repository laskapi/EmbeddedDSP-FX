#include "Device/Demo/DemoManifestProvider.h"

using namespace Host::Models;

namespace Host::Device::Demo {

DeviceManifest getDemoManifest() {
    DeviceManifest manifest;
    manifest.slotCount = 4;

    // Match firmware EmptyEffect / Delay / Overdrive ids and param metadata.
    EffectSpec none;
    none.id = 0;
    none.name = QStringLiteral("None");
    manifest.availableEffects[none.id] = none;

    EffectSpec delay;
    delay.id = 1;
    delay.name = QStringLiteral("Delay");
    delay.params = {
        {QStringLiteral("Time"), 0.01f, 1.0f, 0.5f},
        {QStringLiteral("Feedback"), 0.0f, 0.95f, 0.3f},
        {QStringLiteral("Mix"), 0.0f, 1.0f, 0.4f}
    };
    manifest.availableEffects[delay.id] = delay;

    EffectSpec overdrive;
    overdrive.id = 2;
    overdrive.name = QStringLiteral("Overdrive");
    overdrive.params = {
        {QStringLiteral("Gain"), 1.0f, 20.0f, 5.0f},
        {QStringLiteral("Tone"), 0.1f, 1.0f, 0.5f}
    };
    manifest.availableEffects[overdrive.id] = overdrive;

    return manifest;
}

} // namespace Host::Device::Demo
