#pragma once

#include <atomic>

class Lock
{
public:
    Lock(): _counter(0) {}

    void lock()
    {
        uint8_t old = 0;

        do {
            // wait while another entity hold the lock.
            while ((old = _counter.load(std::memory_order_acquire)) > 0)
                pause();

        } while (!_counter.compare_exchange_weak(old, 1, std::memory_order_release));
    }

    void unlock()
    {
        _counter.store(0, std::memory_order_release);
    }

private:
    static void pause()
    {
        __asm__ __volatile__("pause\n": : :"memory");
    }

    std::atomic<uint8_t> _counter;
};
