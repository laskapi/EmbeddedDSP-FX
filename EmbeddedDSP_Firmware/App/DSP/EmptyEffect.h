#ifndef EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H

#include <array>
#include <cstdint>

class EmptyEffect {
public:
    static constexpr uint8_t Id = 0;
    static constexpr const char* Name = "Empty";
    
    struct ParamInfo { const char* name; float min; float max; float defaultValue; };
    static constexpr std::array<ParamInfo, 0> Params = {};
    static constexpr uint8_t ParamCount = 0;

    void prepare(float) noexcept {}
    void process(float&, float&) noexcept {}
    void toggleBypass() noexcept {}
    [[nodiscard]] bool isBypassed() const noexcept { return true; }
    void setParamValue(uint8_t, float) noexcept {}
    [[nodiscard]] float getParamValue(uint8_t) const noexcept { return 0.0f; }
};

#endif