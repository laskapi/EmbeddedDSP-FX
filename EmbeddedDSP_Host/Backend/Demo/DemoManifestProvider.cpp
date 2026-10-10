#include "DemoManifestProvider.h"

using namespace Host::Models;

namespace Host::Backend::Demo {

DeviceManifest getDemoManifest() {
    DeviceManifest manifest;
    manifest.slotCount = 4;

    // 1. Distortion
    EffectSpec distortion;
    distortion.id = 1;
    distortion.name = "Overdrive";
    distortion.params = {
        {"Gain", 0.0f, 100.0f, 45.0f},
        {"Tone", 200.0f, 5000.0f, 2500.0f},
        {"Level", 0.0f, 1.0f, 0.7f}
    };
    manifest.availableEffects[distortion.id] = distortion;

    // 2. Delay
    EffectSpec delay;
    delay.id = 2;
    delay.name = "Stereo Delay";
    delay.params = {
        {"Time", 10.0f, 1000.0f, 350.0f},
        {"Feedback", 0.0f, 0.95f, 0.4f},
        {"Mix", 0.0f, 1.0f, 0.3f}
    };
    manifest.availableEffects[delay.id] = delay;

    // 3. EQ
    EffectSpec eq;
    eq.id = 3;
    eq.name = "3-Band EQ";
    eq.params = {
        {"Bass", -12.0f, 12.0f, 0.0f},
        {"Middle", -12.0f, 12.0f, 2.0f},
        {"Treble", -12.0f, 12.0f, -1.0f}
    };
    manifest.availableEffects[eq.id] = eq;

    // 4. Reverb
    EffectSpec reverb;
    reverb.id = 4;
    reverb.name = "Hall Reverb";
    reverb.params = {
        {"Decay", 0.1f, 5.0f, 2.4f},
        {"Damping", 0.0f, 1.0f, 0.5f},
        {"Mix", 0.0f, 1.0f, 0.25f}
    };
    manifest.availableEffects[reverb.id] = reverb;

    return manifest;
}

} // namespace Host::Backend::Demo
