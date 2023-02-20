#include <stddef.h>

#define COMPILER_BARRIER() __asm__ __volatile__("" : : : "memory")
#define CPU_RELAX() __asm__ __volatile__("pause\n": : :"memory")
#define CAS(address, exp, target) __sync_bool_compare_and_swap(address, exp, target)
#define ATOMIC_EXCHANGE(address, val) __atomic_exchange_n(address, val, __ATOMIC_SEQ_CST)
#define CPU_BARRIER() __sync_synchronize()

typedef struct mcs_lock_node {
    volatile int waiting;
    struct mcs_lock_node *volatile next;
} mcs_lock_node;

typedef mcs_lock_node *volatile mcs_lock;

static mcs_lock_node *get_my_mcs_node()
{
    static __thread mcs_lock_node my_mcs_node;
    return &my_mcs_node;
}

void spin_lock(mcs_lock *lock)
{
    mcs_lock_node *me = get_my_mcs_node();
    me->next = NULL;

    mcs_lock_node *pre = ATOMIC_EXCHANGE(lock, me);

    if (pre == NULL) {
        return;
    }

    me->waiting = 1;
    COMPILER_BARRIER();
    pre->next = me;

    while (me->waiting) {
        CPU_RELAX();
    }

    CPU_BARRIER();
}

void spin_unlock(mcs_lock *lock)
{
    CPU_BARRIER();

    mcs_lock_node *me = get_my_mcs_node();

    if (me->next == NULL) {
        if (CAS(lock, me, NULL)) {
            return;
        }

        // execute here means someone has setup the lock value(after line 27)
        // need to wait for it to setup the lock node link.
        while (me->next == NULL) {
            CPU_RELAX();
        }
    }

    me->next->waiting = 0;
}