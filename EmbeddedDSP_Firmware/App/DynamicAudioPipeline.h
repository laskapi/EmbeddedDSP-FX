#ifndef DYNAMICAUDIOPIPELINE_H
#define DYNAMICAUDIOPIPELINE_H

#include "AudioEffectConcept.h"
#include "DelayEffect.h"

#include <array>
#include <cstddef>
#include <utility>
#include <variant>

/**
 * @brief Null Object Pattern implementation for bypassed or unassigned slots.
 *
 * Satisfies the AudioEffect concept with no-op execution.
 */
class EmptyEffect {
public:
    void process(float&, float&) noexcept {}
    void toggleBypass() noexcept {}
    [[nodiscard]] bool isBypassed() const noexcept { return true; }
};

/**
 * @brief Type alias containing all available Audio Processing Units in the system.
 */
using EffectVariant = std::variant<EmptyEffect, DelayEffect /*, OverdriveEffect */>;

/**
 * @brief Static-polymorphic audio processing pipeline using std::variant and std::visit.
 *
 * Provides dynamic slot assignment without dynamic memory allocation (Zero-Heap)
 * or VTable function pointer jumps (Zero-Overhead Inlining).
 *
 * @tparam NumSlots Total number of available processing slots in the chain.
 */
template <std::size_t NumSlots = 4>
class DynamicAudioPipeline {
public:
    DynamicAudioPipeline() = default;

    /**
     * @brief Processes input audio channels sequentially through all active pipeline slots.
     * @param left Reference to left channel float sample [-1.0f, 1.0f].
     * @param right Reference to right channel float sample [-1.0f, 1.0f].
     */
    void process(float& left, float& right) noexcept {
        for (auto& slot : slots) {
            std::visit([&left, &right](auto& fx) {
                if (!fx.isBypassed()) {
                    fx.process(left, right);
                }
            }, slot);
        }
    }

    /**
     * @brief Assigns a concrete audio effect instance to a specified pipeline slot.
     * @tparam T Type satisfying the AudioEffect concept.
     * @param slotIndex Target slot index [0, NumSlots - 1].
     * @param effect Effect instance to move/copy into the slot.
     */
    template <AudioEffect T>
    void setEffectInSlot(std::size_t slotIndex, T&& effect) {
        if (slotIndex < NumSlots) {
            slots[slotIndex] = std::forward<T>(effect);
        }
    }

    /**
     * @brief Retrieves a reference to a slot's Variant for parameter modification.
     * @param slotIndex Target slot index.
     * @return Reference to the EffectVariant at specified index.
     */
    [[nodiscard]] EffectVariant& getSlot(std::size_t slotIndex) {
        return slots[slotIndex];
    }

private:
    std::array<EffectVariant, NumSlots> slots{};
};

#endif // DYNAMICAUDIOPIPELINE_H