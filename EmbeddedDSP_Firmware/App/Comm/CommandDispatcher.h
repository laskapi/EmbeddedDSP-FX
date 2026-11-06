#ifndef EMBEDDEDDSP_FIRMWARE_COMMAND_DISPATCHER_H
#define EMBEDDEDDSP_FIRMWARE_COMMAND_DISPATCHER_H

#include "Comm/UsbTx.h"
#include "DSP/FxChain.h"
#include "Protocol/ControlPacket.h"

namespace App::Comm {

/// @brief Routes host→device control packets to FxChain or UsbTx (GetState).
class CommandDispatcher {
public:
    CommandDispatcher(Dsp::FxChain& chain, UsbTx& usbTx) noexcept
        : m_chain(chain)
        , m_usbTx(usbTx) {}

    void dispatch(const Protocol::ControlPacket& pkt) noexcept {
        const uint8_t slotId = pkt.slotId;
        switch (pkt.command) {
            case Protocol::Command::SetParam:
                if (slotId < Dsp::FxChain::kMaxEffectSlots) {
                    std::visit([id = pkt.paramId, val = pkt.value()](auto& fx) {
                        fx.setParamValue(id, val);
                    }, m_chain.slot(slotId));
                }
                break;

            case Protocol::Command::SetEffectType:
                m_chain.setEffectByIndex(slotId, pkt.paramId);
                break;

            case Protocol::Command::BypassToggle:
                if (slotId < Dsp::FxChain::kMaxEffectSlots) {
                    std::visit([](auto& fx) { fx.toggleBypass(); }, m_chain.slot(slotId));
                }
                break;

            case Protocol::Command::ClearSlot:
                m_chain.clearSlot(slotId);
                break;

            case Protocol::Command::SwapSlots:
                m_chain.swapSlots(slotId, pkt.paramId);
                break;

            case Protocol::Command::SetActiveSlots:
                // v1: fixed chain size; EmptyEffect marks unused slots. Ignore.
                break;

            case Protocol::Command::SetMasterVolume:
                m_chain.setMasterGain(pkt.value());
                break;

            case Protocol::Command::GetState:
                m_usbTx.beginSync(m_chain);
                break;

            default:
                break;
        }
    }

private:
    Dsp::FxChain& m_chain;
    UsbTx& m_usbTx;
};

} // namespace App::Comm

#endif // EMBEDDEDDSP_FIRMWARE_COMMAND_DISPATCHER_H
