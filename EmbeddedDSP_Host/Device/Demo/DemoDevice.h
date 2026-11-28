#ifndef EMBEDDEDDSP_HOST_DEMO_DEVICE_H
#define EMBEDDEDDSP_HOST_DEMO_DEVICE_H

#include "Device/IDevice.h"
#include "Protocol/ProtocolCommon.h"
#include <array>
#include <memory>
#include <optional>

namespace Host::Device {

namespace Demo { class DemoAudioSimulator; }

/// @brief Offline IDevice: GetState + Set* Report* echoes (parity with MCU UsbTx).
class DemoDevice : public IDevice {
    Q_OBJECT
public:
    /// @brief Matches App::Dsp::kMaxEffectSlots on firmware.
    static constexpr int kSlotCount = 6;
    static constexpr int kPresetCount = Protocol::kUserPresetCount;

    explicit DemoDevice(QObject* parent = nullptr);
    ~DemoDevice() override;

    bool open(const QString& connectionId) override;
    void close() override;
    bool writeControlPacket(const Protocol::ControlPacket& pkt) override;

private:
    static constexpr int kMaxParams = 8;

    struct SlotState {
        int effectTypeId{0};
        bool bypassed{false};
        std::array<float, kMaxParams> params{};
    };

    struct PresetSnapshot {
        float masterGain{0.0f};
        /// @brief Avoid member name `slots` — Qt moc treats it as a keyword.
        std::array<SlotState, kSlotCount> slotStates{};
    };

    void emitSlotReports(int slotIndex);
    void emitBypassReport(int slotIndex);
    void emitMasterVolumeReport();
    void emitActivePresetReport();
    void emitFullState();
    /// @brief Slot/master/activePreset reports only (no manifest — keeps Host rack intact).
    void emitChainState();
    void resetSlotsToEmpty();
    void applyEffectDefaults(int slotIndex, int effectTypeId);
    void savePreset(int presetId);
    [[nodiscard]] bool loadPreset(int presetId);

    /// @brief Matches App::Dsp::MasterVolume::kDefaultGain (MCU boot default).
    static constexpr float kDefaultMasterGain = 0.8f;

    bool m_isOpen{false};
    float m_masterGain{kDefaultMasterGain};
    int m_activePreset{0};
    std::array<SlotState, kSlotCount> m_slots{};
    std::array<std::optional<PresetSnapshot>, kPresetCount> m_presets{};
    std::unique_ptr<Demo::DemoAudioSimulator> m_simulator;
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_DEMO_DEVICE_H
