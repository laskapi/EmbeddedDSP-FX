#ifndef EMBEDDEDDSP_FIRMWARE_AUDIOEFFECTCONCEPT_H
#define EMBEDDEDDSP_FIRMWARE_AUDIOEFFECTCONCEPT_H

#include <concepts>

/**
 * @brief Concept defining the compile-time static interface for audio processing units.
 * 
 * Enforces zero-overhead polymorphism (no VTable / virtual function calls)
 * for real-time audio DSP pipeline integration on embedded platforms.
 * 
 * @tparam T Effect class type to validate.
 */
template <typename T>
concept AudioEffect = requires(T effect, float& left, float& right) {
    { effect.process(left, right) } -> std::same_as<void>;
    { effect.toggleBypass() }       -> std::same_as<void>;
    { effect.isBypassed() }         -> std::same_as<bool>;
};

#endif // EMBEDDEDDSP_FIRMWARE_AUDIOEFFECTCONCEPT_H