#ifndef EMBEDDEDDSP_HOST_DEMO_DEVICE_H
#define EMBEDDEDDSP_HOST_DEMO_DEVICE_H

#include "Device/IDevice.h"
#include <array>
#include <memory>

namespace Host::Device {

namespace Demo { class DemoAudioSimulator; }

/// @brief Offline IDevice: GetState + Set* Report* echoes (parity with MCU UsbTx).
class DemoDevice : public IDevice {
    Q_OBJECT
public:
    explicit DemoDevice(QObject* parent = nullptr);
    ~DemoDevice() override;

    bool open(const QString& connectionId) override;
    void close() override;
    bool writeControlPacket(const Protocol::ControlPacket& pkt) override;

private:
    static constexpr int kSlotCount = 4;
    static constexpr int kMaxParams = 8;

    struct SlotState {
        int effectTypeId{0};
        bool bypassed{false};
        std::array<float, kMaxParams> params{};
    };

    void emitSlotReports(int slotIndex);
    void emitBypassReport(int slotIndex);
    void emitMasterVolumeReport();
    void resetSlotsToEmpty();
    void applyEffectDefaults(int slotIndex, int effectTypeId);

    /// @brief Matches App::Dsp::MasterVolume::kDefaultGain (MCU boot default).
    static constexpr float kDefaultMasterGain = 0.8f;

    bool m_isOpen{false};
    float m_masterGain{kDefaultMasterGain};
    std::array<SlotState, kSlotCount> m_slots{};
    std::unique_ptr<Demo::DemoAudioSimulator> m_simulator;
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_DEMO_DEVICE_H
