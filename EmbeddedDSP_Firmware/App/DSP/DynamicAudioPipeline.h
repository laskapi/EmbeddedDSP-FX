#ifndef EMBEDDEDDSP_DYNAMIC_AUDIO_PIPELINE_H
#define EMBEDDEDDSP_DYNAMIC_AUDIO_PIPELINE_H

#include "AudioEffectConcept.h"
#include "EmptyEffect.h"
#include "DelayEffect.h"
#include "OverdriveEffect.h"
#include "SharedBuffer.h"
#include <variant>
#include <array>
#include <algorithm>
#include <utility>
#include <cstdio>
#include <string_view>

inline constexpr std::size_t GLOBAL_MAX_SLOTS = 4;

/**
 * @brief DSP state for persistence.
 */
struct PipelineSettings {
    static constexpr uint32_t MAGIC = 0x44535046; // "DSPF"
    
    uint32_t magic;          
    uint8_t activeSlots;     
    
    struct SlotData {
        uint8_t effectId;    
        bool bypassed;
        float params[8];     
    } slots[GLOBAL_MAX_SLOTS];
};

using EffectVariant = std::variant<EmptyEffect, DelayEffect, OverdriveEffect>;

/**
 * @brief Dynamic audio pipeline.
 */
class DynamicAudioPipeline {
public:
    static constexpr std::size_t MAX_AUDIO_SLOTS = GLOBAL_MAX_SLOTS;

private:
    std::array<EffectVariant, MAX_AUDIO_SLOTS> m_slots{};
    std::size_t m_activeSlotsCount{MAX_AUDIO_SLOTS};

public:
    DynamicAudioPipeline() {
        m_slots.fill(EmptyEffect{});
    }

    void prepare(float sampleRate) noexcept {
        for (auto& slot : m_slots) {
            std::visit([sampleRate](auto& fx) { fx.prepare(sampleRate); }, slot);
        }
    }

    void process(float& left, float& right) noexcept {
        for (std::size_t i = 0; i < m_activeSlotsCount; ++i) {
            std::visit([&left, &right](auto& fx) {
                if (!fx.isBypassed()) {
                    fx.process(left, right);
                }
            }, m_slots[i]);
        }
    }

    void setEffectByIndex(std::size_t slotId, std::size_t targetId, float sampleRate) {
        if (slotId >= MAX_AUDIO_SLOTS) return;

        auto trySet = [&]<typename T>() {
            if (T::Id == targetId) { 
                T newEffect{};
                newEffect.prepare(sampleRate);
                m_slots[slotId] = std::move(newEffect);
                return true; 
            }
            return false;
        };

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (trySet.template operator()<std::variant_alternative_t<Is, EffectVariant>>() || ...);
        }(std::make_index_sequence<std::variant_size_v<EffectVariant>>{});
    }

    [[nodiscard]] PipelineSettings captureSettings() const {
        PipelineSettings s;
        s.magic = PipelineSettings::MAGIC;
        s.activeSlots = static_cast<uint8_t>(m_activeSlotsCount);

        for (uint8_t i = 0; i < MAX_AUDIO_SLOTS; ++i) {
            std::visit([&s, i](auto& fx) {
                using T = std::decay_t<decltype(fx)>;
                s.slots[i].effectId = T::Id;
                s.slots[i].bypassed = fx.isBypassed();
                
                std::fill(std::begin(s.slots[i].params), std::end(s.slots[i].params), 0.0f);
                for (uint8_t p = 0; p < T::ParamCount && p < 8; ++p) {
                    s.slots[i].params[p] = fx.getParamValue(p);
                }
            }, m_slots[i]);
        }
        return s;
    }

    void applySettings(const PipelineSettings& s, float sampleRate) {
        if (s.magic != PipelineSettings::MAGIC) return;
        
        m_activeSlotsCount = std::min((std::size_t)s.activeSlots, MAX_AUDIO_SLOTS);
        for (uint8_t i = 0; i < MAX_AUDIO_SLOTS; ++i) {
            setEffectByIndex(i, s.slots[i].effectId, sampleRate);
            
            std::visit([&s, i](auto& fx) {
                if (s.slots[i].bypassed != fx.isBypassed()) fx.toggleBypass();
                for (uint8_t p = 0; p < 8; ++p) {
                    fx.setParamValue(p, s.slots[i].params[p]);
                }
            }, m_slots[i]);
        }
    }

    /**
     * @brief Generates text manifest for discovery.
     */
    std::string_view generateGlobalManifest() {
        char* const buf = reinterpret_cast<char*>(SharedBuffer::s_buffer);
        size_t offset = 0;
        const size_t maxSize = SharedBuffer::Capacity;

        int written = snprintf(buf + offset, maxSize - offset, "CONF:SLOTS=%d\n", (int)MAX_AUDIO_SLOTS);
        if (written > 0 && (size_t)written < (maxSize - offset)) {
            offset += written;
        }

        auto processEffect = [&]<typename T>() {
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
        if (slotIndex < MAX_AUDIO_SLOTS) m_slots[slotIndex] = EmptyEffect{};
    }

    void swapSlots(std::size_t a, std::size_t b) noexcept {
        if (a < MAX_AUDIO_SLOTS && b < MAX_AUDIO_SLOTS) std::swap(m_slots[a], m_slots[b]);
    }

    void setActiveSlotsCount(std::size_t count) noexcept {
        m_activeSlotsCount = std::min(count, MAX_AUDIO_SLOTS);
    }

    [[nodiscard]] std::size_t getActiveSlotsCount() const noexcept { return m_activeSlotsCount; }
    [[nodiscard]] EffectVariant& getSlot(std::size_t i) { return m_slots[i]; }
};

#endif // EMBEDDEDDSP_DYNAMIC_AUDIO_PIPELINE_H
