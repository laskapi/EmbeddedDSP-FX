#ifndef EMBEDDEDDSP_DYNAMIC_AUDIO_PIPELINE_H
#define EMBEDDEDDSP_DYNAMIC_AUDIO_PIPELINE_H

#include "AudioEffectConcept.h"
#include "DelayEffect.h"
#include "OverdriveEffect.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>
#include <variant>

/**
 * @brief Global system constraint defining maximum available processing slots.
 */
inline constexpr std::size_t MAX_AUDIO_SLOTS = 4;

/**
 * @brief Null Object Pattern implementation for unassigned or muted slots.
 * Satisfies the AudioEffect concept with zero-overhead execution.
 */
class EmptyEffect {
public:
    void prepare(float) noexcept {}
    void process(float&, float&) noexcept {}
    void toggleBypass() noexcept {}
    [[nodiscard]] bool isBypassed() const noexcept { return true; }
};

/**
 * @brief Type alias containing all available Audio Processing Units in the system.
 */
using EffectVariant = std::variant<EmptyEffect, DelayEffect, OverdriveEffect>;

/**
 * @brief Static-polymorphic audio processing pipeline using std::variant and std::visit.
 *
 * Provides dynamic slot assignment, reordering, and sizing without dynamic
 * memory allocation (Zero-Heap) or virtual dispatch overhead (Zero-Overhead).
 */
class DynamicAudioPipeline {
private:
    std::array<EffectVariant, MAX_AUDIO_SLOTS> m_slots{};
    std::size_t m_activeSlotsCount{MAX_AUDIO_SLOTS};

public:
    DynamicAudioPipeline() {
        m_slots.fill(EmptyEffect{});
    }

    /**
     * @brief Prepares all effects in the pipeline with current sample rate.
     */
    void prepare(float sampleRate) noexcept {
        for (auto& slot : m_slots) {
            std::visit([sampleRate](auto& fx) {
                fx.prepare(sampleRate);
            }, slot);
        }
    }

    /**
     * @brief Processes input audio channels sequentially through active pipeline slots.
     */
    void process(float& left, float& right) noexcept {
        for (std::size_t i = 0; i < m_activeSlotsCount; ++i) {
            std::visit([&left, &right](auto& fx) {
                if (!fx.isBypassed()) {
                    fx.process(left, right);
                }
            }, m_slots[i]);
        }
    }

    /**
     * @brief Dynamically limits the number of active slots evaluated during audio processing.
     */
    void setActiveSlotsCount(std::size_t count) noexcept {
        m_activeSlotsCount = std::min(count, MAX_AUDIO_SLOTS);
    }

    [[nodiscard]] std::size_t getActiveSlotsCount() const noexcept {
        return m_activeSlotsCount;
    }

    /**
     * @brief Assigns a concrete audio effect instance to a specified pipeline slot.
     */
    template <AudioEffect T>
    void setEffectInSlot(std::size_t slotIndex, T&& effect) {
        if (slotIndex < MAX_AUDIO_SLOTS) {
            m_slots[slotIndex] = std::forward<T>(effect);
        }
    }

    /**
     * @brief Clears a slot by replacing its content with EmptyEffect.
     */
    void clearSlot(std::size_t slotIndex) noexcept {
        if (slotIndex < MAX_AUDIO_SLOTS) {
            m_slots[slotIndex] = EmptyEffect{};
        }
    }

    /**
     * @brief Swaps the effects between two slots (e.g. for reordering chain in Qt GUI).
     */
    void swapSlots(std::size_t slotA, std::size_t slotB) noexcept {
        if (slotA < MAX_AUDIO_SLOTS && slotB < MAX_AUDIO_SLOTS) {
            std::swap(m_slots[slotA], m_slots[slotB]);
        }
    }

    /**
     * @brief Retrieves a reference to a slot's Variant for parameter modification.
     */
    [[nodiscard]] EffectVariant& getSlot(std::size_t slotIndex) {
        return m_slots[slotIndex];
    }

    [[nodiscard]] const EffectVariant& getSlot(std::size_t slotIndex) const {
        return m_slots[slotIndex];
    }
};

#endif // EMBEDDEDDSP_DYNAMIC_AUDIO_PIPELINE_H