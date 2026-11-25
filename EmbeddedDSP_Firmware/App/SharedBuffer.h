#ifndef EMBEDDEDDSP_FIRMWARE_SHARED_BUFFER_H
#define EMBEDDEDDSP_FIRMWARE_SHARED_BUFFER_H

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace App {

/// @brief Shared RAM buffer (delay line / manifest); single owner at a time.
class SharedBuffer {
public:
    static constexpr std::size_t kCapacity = 64 * 1024;

    enum class Owner : uint8_t { None, Delay, Manifest };

    /// @brief Typed view of the buffer (`count` = number of T elements).
    template<typename T>
    struct View {
        T* data{nullptr};
        std::size_t count{0};

        [[nodiscard]] explicit operator bool() const noexcept { return data != nullptr; }
    };

    SharedBuffer() = default;
    SharedBuffer(const SharedBuffer&) = delete;
    SharedBuffer& operator=(const SharedBuffer&) = delete;
    SharedBuffer(SharedBuffer&&) = delete;
    SharedBuffer& operator=(SharedBuffer&&) = delete;

    /// @brief Claims ownership and returns a typed view, or empty View on failure.
    /// Manifest always succeeds (preempts Delay). Delay succeeds if None or already Delay.
    template<typename T = std::uint8_t>
    [[nodiscard]] View<T> acquire(Owner owner) noexcept {
        static_assert(alignof(T) <= 4, "SharedBuffer alignment is 4");
        if (owner == Owner::None) {
            return {};
        }
        if (owner == Owner::Manifest) {
            m_owner = Owner::Manifest;
            return makeView<T>();
        }
        if (owner == Owner::Delay
            && (m_owner == Owner::None || m_owner == Owner::Delay)) {
            m_owner = Owner::Delay;
            return makeView<T>();
        }
        return {};
    }

    /// @brief Releases only if @p owner is the current holder → None.
    void release(Owner owner) noexcept {
        if (m_owner == owner) {
            m_owner = Owner::None;
        }
    }

private:
    template<typename T>
    [[nodiscard]] View<T> makeView() noexcept {
        return View<T>{
            reinterpret_cast<T*>(m_buf),
            kCapacity / sizeof(T),
        };
    }

    alignas(4) std::uint8_t m_buf[kCapacity]{};
    Owner m_owner{Owner::None};
};

/// @brief Process-wide shared buffer. Do not create additional SharedBuffer instances.
inline SharedBuffer g_sharedBuffer{};

} // namespace App

#endif // EMBEDDEDDSP_FIRMWARE_SHARED_BUFFER_H
