#ifndef EMBEDDEDDSP_FIRMWARE_SPSC_QUEUE_H
#define EMBEDDEDDSP_FIRMWARE_SPSC_QUEUE_H

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>

namespace App {

/// @brief Lock-free SPSC queue; Capacity must be a power of 2.
template <typename T, size_t Capacity>
class SpscQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");

public:
    SpscQueue() : m_head(0), m_tail(0) {}

    /// @brief Non-blocking push; false if full.
    bool push(const T& item) {
        size_t head = m_head.load(std::memory_order_relaxed);
        size_t next_head = (head + 1) & (Capacity - 1);
        if (next_head == m_tail.load(std::memory_order_acquire)) return false;
        m_buffer[head] = item;
        m_head.store(next_head, std::memory_order_release);
        return true;
    }

    /// @brief Reserves a write slot in place; if full, drops the oldest unread item.
    [[nodiscard]] T* acquireWrite() noexcept {
        if (m_writeIndex.has_value()) {
            return &m_buffer[*m_writeIndex];
        }

        const size_t head = m_head.load(std::memory_order_relaxed);
        const size_t mask = Capacity - 1;
        const size_t next_head = (head + 1) & mask;
        size_t tail = m_tail.load(std::memory_order_acquire);
        if (next_head == tail) {
            m_tail.store((tail + 1) & mask, std::memory_order_release);
        }

        m_writeIndex = head;
        return &m_buffer[head];
    }

    /// @brief Publishes the slot from acquireWrite (makes it visible to the reader).
    void publishWrite() noexcept {
        if (!m_writeIndex.has_value()) {
            return;
        }

        const size_t head = *m_writeIndex;
        m_writeIndex.reset();
        m_head.store((head + 1) & (Capacity - 1), std::memory_order_release);
    }

    /// @brief Non-blocking pop; nullopt if empty.
    std::optional<T> pop() {
        size_t tail = m_tail.load(std::memory_order_relaxed);
        if (tail == m_head.load(std::memory_order_acquire)) return std::nullopt;
        T item = m_buffer[tail];
        m_tail.store((tail + 1) & (Capacity - 1), std::memory_order_release);
        return item;
    }

    bool isEmpty() const {
        return m_head.load(std::memory_order_acquire) == m_tail.load(std::memory_order_acquire);
    }

private:
    /// @brief Fixed padding between atomics (avoids -Winterference-size / ABI drift).
    static constexpr std::size_t kCacheLineSize = 64;

    std::array<T, Capacity> m_buffer;
    alignas(kCacheLineSize) std::atomic<size_t> m_head;
    alignas(kCacheLineSize) std::atomic<size_t> m_tail;
    std::optional<size_t> m_writeIndex{};
};

} // namespace App

#endif // EMBEDDEDDSP_FIRMWARE_SPSC_QUEUE_H
