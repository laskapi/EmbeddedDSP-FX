#ifndef EMBEDDEDDSP_FIRMWARE_AUDIOEFFECTCONCEPT_H
#define EMBEDDEDDSP_FIRMWARE_AUDIOEFFECTCONCEPT_H

#include <concepts>

/**
 * @brief Concept defining the compile-time static interface for audio processing units.
 * 
 * Enforces zero-overhead polymorphism (no VTable / virtual function calls)
 * and real-time safety for embedded DSP pipeline integration.
 *
 * @tparam T Effect class type to validate.
 */
template <typename T>
concept AudioEffect = requires(T effect, const T constEffect, float& left, float& right, float sampleRate) {
    // Real-time audio frame processing (in-place stereo)
    { effect.process(left, right) } noexcept -> std::same_as<void>;

    // Hardware/DSP initialization and sample rate configuration
    { effect.prepare(sampleRate) } noexcept -> std::same_as<void>;

    // State control interface
    { effect.toggleBypass() } noexcept -> std::same_as<void>;

    // Const correctness check for isBypassed()
    { constEffect.isBypassed() } noexcept -> std::same_as<bool>;
};

#endif // EMBEDDEDDSP_FIRMWARE_AUDIOEFFECTCONCEPT_H