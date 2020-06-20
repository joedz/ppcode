#pragma once

#include "../util/noncopyable.h"
#include "scopedlock.h"

#include <pthread.h>

namespace ppcode {

// 读写锁
class RWMutex{
public:
    // 局部读写锁
    using ReadLock = ReadScopedLock<RWMutex>;
    using WriteLock = WriteScopedLock<RWMutex>;

    RWMutex();
    ~RWMutex();

    void rdlock();
    void wrlock();

    bool try_rdlock();
    bool try_wrlock();

    void unlock();

    bool time_rdlock(int sec = 1, int msec = 1000);
    bool time_wrlock(int sec = 1, int msec = 1000);

private:
    pthread_rwlock_t m_rwmutex;
};

}