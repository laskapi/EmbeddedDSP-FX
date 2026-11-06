#ifndef EMBEDDEDDSP_FIRMWARE_SHARED_BUFFER_H
#define EMBEDDEDDSP_FIRMWARE_SHARED_BUFFER_H

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace App {

/// @brief Shared RAM buffer (delay line / manifest); single owner at a time.
class SharedBuffer {
public:
    static constexpr std::size_t kCapacity = 32768;

    enum class Owner : uint8_t { None, Delay, Manifest };

    /// @brief Typed view of the buffer (`count` = number of T elements).
    template<typename T>
    struct View {
        T* data{nullptr};
        std::size_t count{0};
    };

    SharedBuffer() = default;
    SharedBuffer(const SharedBuffer&) = delete;
    SharedBuffer& operator=(const SharedBuffer&) = delete;
    SharedBuffer(SharedBuffer&&) = delete;
    SharedBuffer& operator=(SharedBuffer&&) = delete;

    /// @brief Claims the buffer. Manifest always succeeds (preempts Delay).
    /// Delay succeeds only if None or already Delay.
    [[nodiscard]] bool acquire(Owner owner) noexcept {
        if (owner == Owner::None) {
            return false;
        }
        if (owner == Owner::Manifest) {
            m_owner = Owner::Manifest;
            return true;
        }
        if (m_owner == Owner::None || m_owner == Owner::Delay) {
            m_owner = Owner::Delay;
            return true;
        }
        return false;
    }

    /// @brief Releases only if @p owner is the current holder → None.
    void release(Owner owner) noexcept {
        if (m_owner == owner) {
            m_owner = Owner::None;
        }
    }

    /// @brief Access the buffer as @tparam T (default: bytes). Caller must hold @p owner.
    template<typename T = std::uint8_t>
    [[nodiscard]] View<T> get(Owner owner) noexcept {
        static_assert(alignof(T) <= 4, "SharedBuffer alignment is 4");
        assert(owner != Owner::None);
        assert(m_owner == owner);
        return View<T>{
            reinterpret_cast<T*>(m_buf),
            kCapacity / sizeof(T),
        };
    }

private:
    alignas(4) std::uint8_t m_buf[kCapacity]{};
    Owner m_owner{Owner::None};
};

/// @brief Process-wide shared buffer. Do not create additional SharedBuffer instances.
inline SharedBuffer g_sharedBuffer{};

} // namespace App

#endif // EMBEDDEDDSP_FIRMWARE_SHARED_BUFFER_H
