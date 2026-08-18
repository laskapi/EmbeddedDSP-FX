#ifndef EMBEDDEDDSP_LOCKFREEQUEUE_H
#define EMBEDDEDDSP_LOCKFREEQUEUE_H

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>

template <typename T, size_t Capacity>
class LockFreeQueue {
public:
    LockFreeQueue() = default;

    // Wywoływane z przerwania USB (ISR) - dodaje bajt do kolejki
    bool push(const T& item) noexcept {
        const auto currentTail = tail_.load(std::memory_order_relaxed);
        const auto nextTail = (currentTail + 1) % Capacity;

        if (nextTail == head_.load(std::memory_order_acquire)) {
            return false; // Bufor przepełniony
        }

        buffer_[currentTail] = item;
        tail_.store(nextTail, std::memory_order_release);
        return true;
    }

    // Wywoływane w pętli głównej (Main Loop) - wyciąga bajt z kolejki
    [[nodiscard]] std::optional<T> pop() noexcept {
        const auto currentHead = head_.load(std::memory_order_relaxed);

        if (currentHead == tail_.load(std::memory_order_acquire)) {
            return std::nullopt; // Bufor pusty
        }

        T item = buffer_[currentHead];
        head_.store((currentHead + 1) % Capacity, std::memory_order_release);
        return item;
    }

private:
    std::array<T, Capacity> buffer_{};
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};

#endif //EMBEDDEDDSP_LOCKFREEQUEUE_H
