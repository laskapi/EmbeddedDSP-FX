#ifndef EMBEDDEDDSP_FIRMWARE_MANIFEST_BUILDER_H
#define EMBEDDEDDSP_FIRMWARE_MANIFEST_BUILDER_H

#include "DSP/FxChain.h"
#include "SharedBuffer.h"

#include <cstdio>
#include <utility>
#include <variant>

namespace App::Comm {

/// @brief Writes capability manifest (C:/E:/P:) into @p buf; returns length before trailing '\\0'.
[[nodiscard]] inline std::size_t writeManifest(App::SharedBuffer::View<char> buf,
                                               std::size_t slotCount) noexcept
{
    char* const dest = buf.data;
    std::size_t offset = 0;
    const std::size_t maxSize = buf.count;

    int written = std::snprintf(dest + offset, maxSize - offset, "C:SLOTS:%d\n",
                                static_cast<int>(slotCount));
    if (written > 0 && static_cast<std::size_t>(written) < (maxSize - offset)) {
        offset += static_cast<std::size_t>(written);
    }

    auto appendEffect = [&]<typename T>() {
        if constexpr (T::Id == 0) {
            return; // skip EmptyEffect
        }

        written = std::snprintf(dest + offset, maxSize - offset, "E:%d:%s",
                                static_cast<int>(T::Id), T::Name);
        if (written > 0 && static_cast<std::size_t>(written) < (maxSize - offset)) {
            offset += static_cast<std::size_t>(written);
        }

        for (const auto& p : T::Params) {
            written = std::snprintf(dest + offset, maxSize - offset, ";P:%s:%.2f:%.2f:%.2f",
                                    p.name, p.min, p.max, p.defaultValue);
            if (written > 0 && static_cast<std::size_t>(written) < (maxSize - offset)) {
                offset += static_cast<std::size_t>(written);
            }
        }

        if (offset < maxSize - 1) {
            dest[offset++] = '\n';
        }
    };

    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (appendEffect.template operator()<std::variant_alternative_t<Is, App::Dsp::EffectVariant>>(),
         ...);
    }(std::make_index_sequence<std::variant_size_v<App::Dsp::EffectVariant>>{});

    if (offset < maxSize) {
        dest[offset] = '\0';
    }

    return offset;
}

} // namespace App::Comm

#endif // EMBEDDEDDSP_FIRMWARE_MANIFEST_BUILDER_H
