#ifndef EMBEDDEDDSP_SPSC_QUEUE_H
#define EMBEDDEDDSP_SPSC_QUEUE_H

#include <atomic>
#include <optional>
#include <array>

namespace Protocol {

/**
 * @brief Thread-safe Single Producer Single Consumer (SPSC) lock-free queue.
 * 
 * @tparam T Type of items in the queue.
 * @tparam Capacity Maximum number of items (must be power of 2).
 */
template <typename T, size_t Capacity>
class SpscQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");

public:
    SpscQueue() : m_head(0), m_tail(0) {}

    /** @brief Pushes an item to the queue. Non-blocking. */
    bool push(const T& item) {
        size_t head = m_head.load(std::memory_order_relaxed);
        size_t next_head = (head + 1) & (Capacity - 1);
        if (next_head == m_tail.load(std::memory_order_acquire)) return false;
        m_buffer[head] = item;
        m_head.store(next_head, std::memory_order_release);
        return true;
    }

    /** @brief Pops an item from the queue. Non-blocking. */
    std::optional<T> pop() {
        size_t tail = m_tail.load(std::memory_order_relaxed);
        if (tail == m_head.load(std::memory_order_acquire)) return std::nullopt;
        T item = m_buffer[tail];
        m_tail.store((tail + 1) & (Capacity - 1), std::memory_order_release);
        return item;
    }

    /** @return True if the queue is empty. */
    bool isEmpty() const {
        return m_head.load(std::memory_order_acquire) == m_tail.load(std::memory_order_acquire);
    }

private:
    std::array<T, Capacity> m_buffer;
    std::atomic<size_t> m_head;
    std::atomic<size_t> m_tail;
};

} // namespace Protocol

#endif // EMBEDDEDDSP_SPSC_QUEUE_H
