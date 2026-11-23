#ifndef EMBEDDEDDSP_FIRMWARE_COMMAND_DISPATCHER_H
#define EMBEDDEDDSP_FIRMWARE_COMMAND_DISPATCHER_H

#include "Comm/UsbTx.h"
#include "DSP/FxChain.h"
#include "Storage/PresetStore.h"
#include "Protocol/ControlPacket.h"

#include <variant>

namespace App::Comm {

/// @brief Routes host→device control packets to FxChain / UsbTx / PresetStore.
class CommandDispatcher {
public:
    CommandDispatcher(Dsp::FxChain& chain, UsbTx& usbTx, Storage::PresetStore& presets) noexcept
        : m_chain(chain)
        , m_usbTx(usbTx)
        , m_presets(presets) {}

    void dispatch(const Protocol::ControlPacket& pkt) noexcept {
        const uint8_t slotId = pkt.slotId;
        switch (pkt.command) {
            case Protocol::Command::SetParam:
                if (slotId < Dsp::kMaxEffectSlots) {
                    std::visit([id = pkt.paramId, val = pkt.value()](auto& fx) {
                        fx.setParamValue(id, val);
                    }, m_chain.slot(slotId));
                }
                break;

            case Protocol::Command::SetEffectType:
                m_chain.setEffectByIndex(slotId, pkt.effectTypeId);
                m_usbTx.enqueueSlotReports(m_chain, slotId);
                break;

            case Protocol::Command::BypassToggle:
                if (slotId < Dsp::kMaxEffectSlots) {
                    const bool bypassed = pkt.value() > 0.5f;
                    std::visit([bypassed](auto& fx) { fx.setBypassed(bypassed); },
                               m_chain.slot(slotId));
                    m_usbTx.enqueueBypassReport(slotId, bypassed);
                }
                break;

            case Protocol::Command::ClearSlot:
                m_chain.clearSlot(slotId);
                m_usbTx.enqueueSlotReports(m_chain, slotId);
                break;

            case Protocol::Command::MoveEffect: {
                const uint8_t to = pkt.targetSlotId;
                if (slotId >= Dsp::kMaxEffectSlots
                    || to >= Dsp::kMaxEffectSlots) {
                    break;
                }
                m_chain.moveEffect(slotId, to);
                for (uint8_t i = 0; i < Dsp::kMaxEffectSlots; ++i) {
                    m_usbTx.enqueueSlotReports(m_chain, i);
                }
                break;
            }

            case Protocol::Command::SetActiveSlots:
                break;

            case Protocol::Command::SetMasterVolume:
                m_chain.setMasterGain(pkt.value());
                break;

            case Protocol::Command::GetParam:
                if (slotId < Dsp::kMaxEffectSlots) {
                    const uint8_t paramId = pkt.paramId;
                    float value = 0.0f;
                    std::visit([paramId, &value](auto& fx) {
                        value = fx.paramValue(paramId);
                    }, m_chain.slot(slotId));
                    m_usbTx.enqueueParamReport(slotId, paramId, value);
                }
                break;

            case Protocol::Command::GetMasterVolume:
                m_usbTx.enqueueMasterVolumeReport(m_chain.masterGain());
                break;

            case Protocol::Command::GetState:
                m_usbTx.beginSync(m_chain, m_presets.activePreset());
                break;

            case Protocol::Command::SavePreset:
                if (m_presets.save(slotId, m_chain.captureSettings())) {
                    m_usbTx.enqueueActivePresetReport(m_presets.activePreset());
                }
                break;

            case Protocol::Command::LoadPreset: {
                Dsp::ChainSettings settings{};
                if (m_presets.load(slotId, settings) && m_chain.applySettings(settings)) {
                    m_presets.setActivePreset(slotId);
                    m_usbTx.enqueueAllReports(m_chain, slotId);
                }
                break;
            }

            default:
                break;
        }
    }

private:
    Dsp::FxChain& m_chain;
    UsbTx& m_usbTx;
    Storage::PresetStore& m_presets;
};

} // namespace App::Comm

#endif // EMBEDDEDDSP_FIRMWARE_COMMAND_DISPATCHER_H
