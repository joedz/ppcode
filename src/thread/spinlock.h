#pragma once

#include <pthread.h>
#include "../util/noncopyable.h"
#include "scopedlock.h"

namespace ppcode {

// 自旋锁
class Spinlock{
public:
    using Lock = ScopedLock<Spinlock>;

    Spinlock();
    ~Spinlock();
    
    void lock();
    void unlock();

    void trylock();
    
private:
    pthread_spinlock_t m_spinlock;

};
}