#ifndef EMBEDDEDDSP_FIRMWARE_FX_CHAIN_H
#define EMBEDDEDDSP_FIRMWARE_FX_CHAIN_H

#include "DSP/MasterVolume.h"
#include "DSP/AudioEffectConcept.h"
#include "DSP/ChainSettings.h"
#include "DSP/EmptyEffect.h"
#include "DSP/DelayEffect.h"
#include "DSP/OverdriveEffect.h"
#include "DSP/TremoloEffect.h"
#include "DSP/BitcrusherEffect.h"
#include "DSP/EqEffect.h"
#include <variant>
#include <array>
#include <algorithm>
#include <cstddef>
#include <utility>

namespace App::Dsp {

using EffectVariant = std::variant<EmptyEffect,
                                   DelayEffect,
                                   OverdriveEffect,
                                   TremoloEffect,
                                   BitcrusherEffect,
                                   EqEffect>;

/// @brief FX slots plus fixed master volume as the last processing step.
class FxChain {
public:
    explicit FxChain(float sampleRate) noexcept
        : m_sampleRate(sampleRate) {
        m_master.updateSampleRate(m_sampleRate);
    }

    /// @brief Propagates a new sample rate to every slot and master (runtime SR change).
    void updateSampleRate(float sampleRate) noexcept {
        m_sampleRate = sampleRate;
        for (auto& slot : m_slots) {
            std::visit([this](auto& fx) { fx.updateSampleRate(m_sampleRate); }, slot);
        }
        m_master.updateSampleRate(m_sampleRate);
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
        if (slotId >= kMaxEffectSlots) {
            return;
        }
 /// @details At most one Delay (shared delay-line RAM); second request is ignored.
        if (effectId == DelayEffect::Id) {
            for (std::size_t i = 0; i < kMaxEffectSlots; ++i) {
                if (i == slotId) {
                    continue;
                }
                if (std::holds_alternative<DelayEffect>(m_slots[i])) {
                    return;
                }
            }
        }

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

    /// @brief Snapshots current chain into ChainSettings.
    [[nodiscard]] ChainSettings captureSettings() const {
        ChainSettings s{};
        s.magic = ChainSettings::kMagic;
        s.version = ChainSettings::kVersion;
        s.slotCount = static_cast<uint8_t>(kMaxEffectSlots);
        s.masterGain = m_master.gain();

        for (uint8_t i = 0; i < kMaxEffectSlots; ++i) {
            std::visit([&s, i](auto& fx) {
                using T = std::decay_t<decltype(fx)>;
                s.slots[i].effectId = T::Id;
                s.slots[i].bypassed = fx.isBypassed() ? 1u : 0u;

                std::fill(std::begin(s.slots[i].params), std::end(s.slots[i].params), 0.0f);
                for (uint8_t p = 0; p < T::ParamCount && p < kMaxEffectParams; ++p) {
                    s.slots[i].params[p] = fx.paramValue(p);
                }
            }, m_slots[i]);
        }
        return s;
    }

    /// @brief Restores chain from ChainSettings.
    [[nodiscard]] bool applySettings(const ChainSettings& s) {
        if (s.magic != ChainSettings::kMagic || s.version != ChainSettings::kVersion) {
            return false;
        }

        for (uint8_t i = 0; i < kMaxEffectSlots; ++i) {
            setEffectByIndex(i, s.slots[i].effectId);

            std::visit([&s, i](auto& fx) {
                fx.setBypassed(s.slots[i].bypassed != 0);
                for (uint8_t p = 0; p < kMaxEffectParams; ++p) {
                    fx.setParamValue(p, s.slots[i].params[p]);
                }
            }, m_slots[i]);
        }
        m_master.setGain(s.masterGain);
        return true;
    }

    void clearSlot(std::size_t slotIndex) noexcept {
        if (slotIndex < kMaxEffectSlots) {
            m_slots[slotIndex] = EmptyEffect{m_sampleRate};
        }
    }

    /// @brief Moves effect at @p from to index @p to.
    void moveEffect(std::size_t from, std::size_t to) noexcept {
        if (from >= kMaxEffectSlots || to >= kMaxEffectSlots || from == to) {
            return;
        }
        if (from < to) {
            std::rotate(m_slots.begin() + static_cast<std::ptrdiff_t>(from),
                        m_slots.begin() + static_cast<std::ptrdiff_t>(from + 1),
                        m_slots.begin() + static_cast<std::ptrdiff_t>(to + 1));
        } else {
            std::rotate(m_slots.begin() + static_cast<std::ptrdiff_t>(to),
                        m_slots.begin() + static_cast<std::ptrdiff_t>(from),
                        m_slots.begin() + static_cast<std::ptrdiff_t>(from + 1));
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
