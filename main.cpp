#include "rwlock.h"

#include <iostream>
#include <thread>
#include <vector>

int counter = 0;

void add(RWLock *lock)
{
    std::cout << "write start\n";
    lock->wrlock();
    ++counter;
    lock->wrunlock();
    std::cout << "write finish\n";
}

void print(RWLock *lock)
{
    std::cout << "read start\n";
    lock->rdlock();
    std::cout << "counter: " << counter << "\n";
    lock->rdunlock();
    std::cout << "read finish\n";
}

int main(void)
{
    RWLock lock;
    std::vector<std::thread> v;

    for (int i = 0; i < 5; ++i) {
        v.push_back(std::thread(add, &lock));
        v.push_back(std::thread(print, &lock));
    }

    for (size_t i = 0; i < v.size(); ++i)
        v[i].join();

	std::cout << "final counter: " << counter << std::endl;

    return 0;
}
