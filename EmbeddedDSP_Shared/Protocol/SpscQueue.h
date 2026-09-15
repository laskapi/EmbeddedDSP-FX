#ifndef EMBEDDEDDSP_PROTOCOL_SPSC_QUEUE_H
#define EMBEDDEDDSP_PROTOCOL_SPSC_QUEUE_H

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>

namespace Protocol {

    template <typename T, std::size_t Capacity>
    class SpscQueue {
    private:
        alignas(64) std::atomic<std::size_t> m_head{0};
        alignas(64) std::atomic<std::size_t> m_tail{0};
        std::array<T, Capacity> m_buffer{};

    public:
        constexpr SpscQueue() noexcept = default;

        bool push(const T& item) noexcept {
            const auto currentTail = m_tail.load(std::memory_order_relaxed);
            const auto nextTail = (currentTail + 1) % Capacity;

            if (nextTail == m_head.load(std::memory_order_acquire)) {
                return false; 
            }

            m_buffer[currentTail] = item;
            m_tail.store(nextTail, std::memory_order_release);
            return true;
        }

        std::optional<T> pop() noexcept {
            const auto currentHead = m_head.load(std::memory_order_relaxed);

            if (currentHead == m_tail.load(std::memory_order_acquire)) {
                return std::nullopt;
            }

            T item = m_buffer[currentHead];
            m_head.store((currentHead + 1) % Capacity, std::memory_order_release);
            return item;
        }

        [[nodiscard]] bool isEmpty() const noexcept {
            return m_head.load(std::memory_order_acquire) == m_tail.load(std::memory_order_acquire);
        }
    };
}

#endif // EMBEDDEDDSP_PROTOCOL_SPSC_QUEUE_H
