#ifndef EMBEDDEDDSP_EFFECT_CHAIN_H
#define EMBEDDEDDSP_EFFECT_CHAIN_H

#include "DSP/AudioEffectConcept.h"
#include "DSP/EmptyEffect.h"
#include "DSP/DelayEffect.h"
#include "DSP/OverdriveEffect.h"
#include "SharedBuffer.h"
#include "Protocol/ProtocolCommon.h"
#include <variant>
#include <array>
#include <algorithm>
#include <utility>
#include <cstdio>
#include <string_view>

using EffectVariant = std::variant<EmptyEffect, DelayEffect, OverdriveEffect>;

/// @brief Fixed-size effect chain; unused slots hold EmptyEffect.
class EffectChain {
public:
    /// @brief Firmware chain capacity (emitted as manifest C:SLOTS).
    static constexpr std::size_t MAX_EFFECT_SLOTS = 4;

    /// @brief Persisted chain state (flash-friendly POD; not wired to flash yet).
    struct Settings {
        static constexpr uint32_t MAGIC = 0x44535046; // "DSPF" — DSP FX blob tag
        uint32_t magic;
        uint8_t slotCount; // always MAX_EFFECT_SLOTS in v1
        struct SlotData {
            uint8_t effectId;
            bool bypassed;
            float params[MAX_EFFECT_PARAMS];
        } slots[MAX_EFFECT_SLOTS];
    };

    explicit EffectChain(float sampleRate) noexcept
        : m_sampleRate(sampleRate) {}

    /// @brief Propagates a new sample rate to every slot (runtime SR change).
    void updateSampleRate(float sampleRate) noexcept {
        m_sampleRate = sampleRate;
        for (auto& slot : m_slots) {
            std::visit([this](auto& fx) { fx.updateSampleRate(m_sampleRate); }, slot);
        }
    }

    /// @brief Runs all non-bypassed slot effects on one stereo sample.
    void process(float& left, float& right) noexcept {
        for (auto& slot : m_slots) {
            std::visit([&left, &right](auto& fx) {
                if (!fx.isBypassed()) {
                    fx.process(left, right);
                }
            }, slot);
        }
    }

    /// @brief Replaces the effect in a slot by effect id.
    void setEffectByIndex(std::size_t slotId, std::size_t effectId) {
        if (slotId >= MAX_EFFECT_SLOTS) return;

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
        s.magic = Settings::MAGIC;
        s.slotCount = static_cast<uint8_t>(MAX_EFFECT_SLOTS);

        for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i) {
            std::visit([&s, i](auto& fx) {
                using T = std::decay_t<decltype(fx)>;
                s.slots[i].effectId = T::Id;
                s.slots[i].bypassed = fx.isBypassed();

                std::fill(std::begin(s.slots[i].params), std::end(s.slots[i].params), 0.0f);
                for (uint8_t p = 0; p < T::ParamCount && p < MAX_EFFECT_PARAMS; ++p) {
                    s.slots[i].params[p] = fx.paramValue(p);
                }
            }, m_slots[i]);
        }
        return s;
    }

    /// @brief Restores chain from Settings (ignores bad magic).
    void applySettings(const Settings& s) {
        if (s.magic != Settings::MAGIC) return;

        for (uint8_t i = 0; i < MAX_EFFECT_SLOTS; ++i) {
            setEffectByIndex(i, s.slots[i].effectId);

            std::visit([&s, i](auto& fx) {
                if (s.slots[i].bypassed != fx.isBypassed()) fx.toggleBypass();
                for (uint8_t p = 0; p < MAX_EFFECT_PARAMS; ++p) {
                    fx.setParamValue(p, s.slots[i].params[p]);
                }
            }, m_slots[i]);
        }
    }

    /// @brief Builds host manifest text into SharedBuffer (C:/E:/P: lines).
    std::string_view generateGlobalManifest() {
        char* const buf = reinterpret_cast<char*>(SharedBuffer::s_buffer);
        size_t offset = 0;
        const size_t maxSize = SharedBuffer::Capacity;

        int written = snprintf(buf + offset, maxSize - offset, "C:SLOTS:%d\n",
                               (int)MAX_EFFECT_SLOTS);
        if (written > 0 && (size_t)written < (maxSize - offset)) {
            offset += written;
        }

        auto processEffect = [&]<typename T>() {
            if constexpr (T::Id == 0) return; // skip EmptyEffect

            int written = snprintf(buf + offset, maxSize - offset, "E:%d:%s", (int)T::Id, T::Name);
            if (written > 0 && (size_t)written < (maxSize - offset)) {
                offset += written;
            }

            for (const auto& p : T::Params) {
                written = snprintf(buf + offset, maxSize - offset, ";P:%s:%.2f:%.2f:%.2f",
                                   p.name, p.min, p.max, p.defaultValue);
                if (written > 0 && (size_t)written < (maxSize - offset)) {
                    offset += written;
                }
            }

            if (offset < maxSize - 1) {
                buf[offset++] = '\n';
            }
        };

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (processEffect.template operator()<std::variant_alternative_t<Is, EffectVariant>>(), ...);
        }(std::make_index_sequence<std::variant_size_v<EffectVariant>>{});

        if (offset < maxSize) {
            buf[offset] = '\0';
        }

        return std::string_view(reinterpret_cast<const char*>(SharedBuffer::s_buffer), offset);
    }

    void clearSlot(std::size_t slotIndex) noexcept {
        if (slotIndex < MAX_EFFECT_SLOTS) {
            m_slots[slotIndex] = EmptyEffect{m_sampleRate};
        }
    }

    void swapSlots(std::size_t a, std::size_t b) noexcept {
        if (a < MAX_EFFECT_SLOTS && b < MAX_EFFECT_SLOTS) {
            std::swap(m_slots[a], m_slots[b]);
        }
    }

    [[nodiscard]] EffectVariant& slot(std::size_t i) { return m_slots[i]; }

private:
    float m_sampleRate;
    std::array<EffectVariant, MAX_EFFECT_SLOTS> m_slots{};
    };

#endif // EMBEDDEDDSP_EFFECT_CHAIN_H
