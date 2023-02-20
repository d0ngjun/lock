#pragma once

#include <atomic>
#include <cstdint>

class RWLock
{
public:
	RWLock(): _counter(0) {}

	void wrlock()
	{
		uint64_t old = 0;
		uint64_t c = 0;

		do {
            // read first, wait for all other read and write operation.
			while ((old = _counter.load(std::memory_order_acquire)) & (WRITE_FLAG | READ_FLAG))
				pause();

			c = old | WRITE_ONE;
		} while (!_counter.compare_exchange_weak(old, c, std::memory_order_release));
	}

	void wrunlock()
	{
		uint64_t old = 0;
		uint64_t c = 0;

		do {
			old = _counter.load(std::memory_order_acquire);
            // this operation is safe even if the lock has not write locked.
			c = old & ~WRITE_ONE;
		} while (!_counter.compare_exchange_weak(old, c, std::memory_order_release));
	}

	void rdlock()
	{
		uint64_t old = 0;
		uint64_t c = 0;

		do {
            // only need to wait other write operation.
			while ((old = _counter.load(std::memory_order_acquire)) & WRITE_FLAG)
				pause();

			c = old + READ_ONE;
		} while (!_counter.compare_exchange_weak(old, c, std::memory_order_release));
	}

	void rdunlock()
	{
		uint64_t old = 0;
		uint64_t c = 0;

		do {
			old = _counter.load(std::memory_order_acquire);

            // not locked by read, protect the write flag.
			if ((old & READ_FLAG) == 0)
				return;

			c = old - READ_ONE;
		} while (!_counter.compare_exchange_weak(old, c, std::memory_order_release));
	}

private:
	static void pause()
	{
		__asm__ __volatile__("pause\n": : :"memory");
	}

	static const uint64_t WRITE_FLAG = 0x8000000000000000;
	static const uint64_t READ_FLAG = 0x7fffffffffffffff;

	static const uint64_t WRITE_ONE = 0x8000000000000000;
	static const uint64_t READ_ONE = 1;

	std::atomic<uint64_t> _counter;
};
