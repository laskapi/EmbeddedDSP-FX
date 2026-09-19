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

/** @brief Binary state for MCU persistence. */
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
 * @brief Core DSP pipeline managing effect slots and audio processing.
 */
class DynamicAudioPipeline {
public:
    static constexpr std::size_t MAX_AUDIO_SLOTS = GLOBAL_MAX_SLOTS;

private:
    std::array<EffectVariant, MAX_AUDIO_SLOTS> m_slots{};
    std::size_t m_activeSlotsCount{MAX_AUDIO_SLOTS};

public:
    DynamicAudioPipeline();

    /** @brief Prepares all effects in the pipeline. */
    void prepare(float sampleRate) noexcept;

    /** @brief Main audio processing loop for a single stereo sample. */
    void process(float& left, float& right) noexcept;

    /** @brief Changes the effect type in a specific slot. */
    void setEffectByIndex(std::size_t slotId, std::size_t targetId, float sampleRate);

    /** @brief Captures the current pipeline state into a binary struct. */
    [[nodiscard]] PipelineSettings captureSettings() const;

    /** @brief Restores the pipeline state from a binary struct. */
    void applySettings(const PipelineSettings& s, float sampleRate);

    /** @brief Generates the text-based discovery manifest. */
    std::string_view generateGlobalManifest();

    /** @brief Resets a slot to EmptyEffect. */
    void clearSlot(std::size_t slotIndex) noexcept;

    /** @brief Swaps contents of two slots. */
    void swapSlots(std::size_t a, std::size_t b) noexcept;

    void setActiveSlotsCount(std::size_t count) noexcept;
    [[nodiscard]] std::size_t getActiveSlotsCount() const noexcept { return m_activeSlotsCount; }
    [[nodiscard]] EffectVariant& getSlot(std::size_t i) { return m_slots[i]; }
};

#endif // EMBEDDEDDSP_DYNAMIC_AUDIO_PIPELINE_H
