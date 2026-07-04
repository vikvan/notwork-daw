#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <vector>

namespace notwork::engine {

// Lock-free single-producer, single-consumer ring buffer.
// Capacity is rounded up to the next power of two so that index masking is a
// single AND. Indices are monotonically increasing size_t — overflow is
// practically impossible.
template <typename T>
class SpscRingBuffer {
public:
    explicit SpscRingBuffer(std::size_t capacity) {
        std::size_t cap = 1;
        while (cap < capacity) cap <<= 1;
        buf_.assign(cap, T{});
        mask_ = cap - 1;
    }

    std::size_t capacity() const { return buf_.size(); }

    std::size_t readAvailable() const {
        return write_.load(std::memory_order_acquire)
             - read_.load(std::memory_order_relaxed);
    }
    std::size_t writeAvailable() const {
        return buf_.size() - readAvailable();
    }

    // Push up to `n` items; returns count actually pushed. Producer thread.
    std::size_t push(const T* src, std::size_t n) {
        const std::size_t w = write_.load(std::memory_order_relaxed);
        const std::size_t r = read_.load(std::memory_order_acquire);
        const std::size_t space   = buf_.size() - (w - r);
        const std::size_t toWrite = std::min(n, space);
        for (std::size_t i = 0; i < toWrite; ++i) {
            buf_[(w + i) & mask_] = src[i];
        }
        write_.store(w + toWrite, std::memory_order_release);
        return toWrite;
    }

    // Pop up to `n` items; returns count actually popped. Consumer thread.
    std::size_t pop(T* dst, std::size_t n) {
        const std::size_t w = write_.load(std::memory_order_acquire);
        const std::size_t r = read_.load(std::memory_order_relaxed);
        const std::size_t avail  = w - r;
        const std::size_t toRead = std::min(n, avail);
        for (std::size_t i = 0; i < toRead; ++i) {
            dst[i] = buf_[(r + i) & mask_];
        }
        read_.store(r + toRead, std::memory_order_release);
        return toRead;
    }

private:
    std::vector<T> buf_;
    std::size_t    mask_ = 0;

    alignas(64) std::atomic<std::size_t> read_{0};
    alignas(64) std::atomic<std::size_t> write_{0};
};

} // namespace notwork::engine
