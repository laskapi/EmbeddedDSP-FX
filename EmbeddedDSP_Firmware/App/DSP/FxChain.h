#ifndef EMBEDDEDDSP_FIRMWARE_FX_CHAIN_H
#define EMBEDDEDDSP_FIRMWARE_FX_CHAIN_H

#include "DSP/MasterVolume.h"
#include "DSP/AudioEffectConcept.h"
#include "DSP/EmptyEffect.h"
#include "DSP/DelayEffect.h"
#include "DSP/OverdriveEffect.h"
#include "Protocol/ProtocolCommon.h"
#include <variant>
#include <array>
#include <algorithm>
#include <utility>

namespace App::Dsp {

using EffectVariant = std::variant<EmptyEffect, DelayEffect, OverdriveEffect>;

/// @brief FX slots plus fixed master volume as the last processing step.
class FxChain {
public:
    /// @brief Firmware chain capacity (emitted as manifest C:SLOTS).
    static constexpr std::size_t kMaxEffectSlots = 4;

    /// @brief Persisted chain state (flash-friendly POD; not wired to flash yet).
    struct Settings {
        static constexpr uint32_t kMagic = 0x44535046; // "DSPF" — DSP FX blob tag
        uint32_t magic;
        uint8_t slotCount; // always kMaxEffectSlots in v1
        struct SlotData {
            uint8_t effectId;
            bool bypassed;
            float params[kMaxEffectParams];
        } slots[kMaxEffectSlots];
    };

    explicit FxChain(float sampleRate) noexcept
        : m_sampleRate(sampleRate) {}

    /// @brief Propagates a new sample rate to every slot (runtime SR change).
    void updateSampleRate(float sampleRate) noexcept {
        m_sampleRate = sampleRate;
        for (auto& slot : m_slots) {
            std::visit([this](auto& fx) { fx.updateSampleRate(m_sampleRate); }, slot);
        }
    }

    /// @brief Runs all non-bypassed slot effects, then master volume.
    void process(float& left, float& right) noexcept {
        for (auto& slot : m_slots) {
            std::visit([&left, &right](auto& fx) {
                if (!fx.isBypassed()) {
                    fx.process(left, right);
                }
            }, slot);
        }
        m_master.apply(left, right);
    }

    [[nodiscard]] float masterGain() const noexcept { return m_master.gain(); }

    void setMasterGain(float gain) noexcept { m_master.setGain(gain); }

    /// @brief Replaces the effect in a slot by effect id.
    void setEffectByIndex(std::size_t slotId, std::size_t effectId) {
        if (slotId >= kMaxEffectSlots) return;

        auto trySet = [&]<typename T>() {
            if (T::Id == effectId) {
                m_slots[slotId] = T{m_sampleRate};
                return true;
            }
            return false;
        };

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (trySet.template operator()<std::variant_alternative_t<Is, EffectVariant>>() || ...);
        }(std::make_index_sequence<std::variant_size_v<EffectVariant>>{});
    }

    /// @brief Snapshots current chain into Settings.
    [[nodiscard]] Settings captureSettings() const {
        Settings s{};
        s.magic = Settings::kMagic;
        s.slotCount = static_cast<uint8_t>(kMaxEffectSlots);

        for (uint8_t i = 0; i < kMaxEffectSlots; ++i) {
            std::visit([&s, i](auto& fx) {
                using T = std::decay_t<decltype(fx)>;
                s.slots[i].effectId = T::Id;
                s.slots[i].bypassed = fx.isBypassed();

                std::fill(std::begin(s.slots[i].params), std::end(s.slots[i].params), 0.0f);
                for (uint8_t p = 0; p < T::ParamCount && p < kMaxEffectParams; ++p) {
                    s.slots[i].params[p] = fx.paramValue(p);
                }
            }, m_slots[i]);
        }
        return s;
    }

    /// @brief Restores chain from Settings.
    void applySettings(const Settings& s) {
        if (s.magic != Settings::kMagic) return;

        for (uint8_t i = 0; i < kMaxEffectSlots; ++i) {
            setEffectByIndex(i, s.slots[i].effectId);

            std::visit([&s, i](auto& fx) {
                fx.setBypassed(s.slots[i].bypassed);
                for (uint8_t p = 0; p < kMaxEffectParams; ++p) {
                    fx.setParamValue(p, s.slots[i].params[p]);
                }
            }, m_slots[i]);
        }
    }

    void clearSlot(std::size_t slotIndex) noexcept {
        if (slotIndex < kMaxEffectSlots) {
            m_slots[slotIndex] = EmptyEffect{m_sampleRate};
        }
    }

    void swapSlots(std::size_t a, std::size_t b) noexcept {
        if (a < kMaxEffectSlots && b < kMaxEffectSlots) {
            std::swap(m_slots[a], m_slots[b]);
        }
    }

    [[nodiscard]] EffectVariant& slot(std::size_t i) { return m_slots[i]; }

private:
    float m_sampleRate;
    std::array<EffectVariant, kMaxEffectSlots> m_slots{};
    MasterVolume m_master{};
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_FX_CHAIN_H
