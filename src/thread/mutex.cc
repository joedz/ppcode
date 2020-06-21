#include "mutex.h"

#include <time.h>

#include "../log.h"

namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();

/******************************************************************
 *  递归锁属性
 * ***************************************************************/

Mutex_Attributes::Mutex_Attributes() {
    int rt = pthread_mutexattr_init(&m_mutex_attr);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutexattr_init failed, rt=" << rt;
        throw std::logic_error("pthread_mutexattr_init failed,");
    }
}
Mutex_Attributes::~Mutex_Attributes() {
    int rt = pthread_mutexattr_destroy(&m_mutex_attr);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutexattr_destroy failed, rt=" << rt;
    }
}

// 因为pthread锁缺省设置为独立进程间不共享，本接口默认设置独立进程间共享
void Mutex_Attributes::setProcessShared(int pshared) {
    int rt = pthread_mutexattr_setpshared(&m_mutex_attr, pshared);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutexattr_setpshared failed, rt=" << rt;
        throw std::logic_error("pthread_mutexattr_setpshared failed,");
    }
}
// 判断是否为进程间共享互斥锁
bool Mutex_Attributes::isProcessShared() {
    int pshared;
    int rt = pthread_mutexattr_getpshared(&m_mutex_attr, &pshared);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutexattr_getpshared failed, rt=" << rt;
        throw std::logic_error("pthread_mutexattr_getpshared failed,");
    }

    if (pshared == PTHREAD_PROCESS_SHARED) {
        return true;
    }

    return false;
}

// 因为pthread锁缺省设置为持有互斥锁进程间终止时不采取任何行动，本接口默认设置为相反操作
void Mutex_Attributes::setRobust(int robust) {
    int rt = pthread_mutexattr_setrobust(&m_mutex_attr, robust);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutexattr_setrobust failed, rt=" << rt;
        throw std::logic_error("pthread_mutexattr_setrobust failed,");
    }
}

// 判断是否在互斥锁进程终止时没有解锁，在其他进程对相同互斥锁加锁时，返回EOWBEDEAD
bool Mutex_Attributes::isRobust() {
    int robust;
    int rt = pthread_mutexattr_getrobust(&m_mutex_attr, &robust);

    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutexattr_getrobust failed, rt=" << rt;
        throw std::logic_error("pthread_mutexattr_getrobust failed,");
    }

    if (robust == PTHREAD_MUTEX_ROBUST) {
        return true;
    }
    return false;
}

void Mutex_Attributes::setMutexType(int type) {
    int rt = pthread_mutexattr_settype(&m_mutex_attr, type);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutexattr_settype failed, rt=" << rt;
        throw std::logic_error("pthread_mutexattr_settype failed,");
    }
}
int Mutex_Attributes::getMutexType() {
    int type;
    int rt = pthread_mutexattr_gettype(&m_mutex_attr, &type);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutexattr_gettype failed, rt=" << rt;
        throw std::logic_error("pthread_mutexattr_gettype failed,");
    }
    return type;
}

/******************************************************************
 *  互斥锁
 * ***************************************************************/

Mutex::Mutex() {
    int rt = pthread_mutex_init(&m_mutex, nullptr);
    if (rt) {
        LOG_ERROR(g_logger) << "phtread_mutex_init failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_lock failed,");
    }
}

Mutex::Mutex(const Mutex_Attributes& attrs) {
    const pthread_mutexattr_t* mutex_attr = attrs.native_handle();
    int rt = pthread_mutex_init(&m_mutex, mutex_attr);
    if (rt) {
        LOG_ERROR(g_logger) << "phtread_mutex_init failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_lock failed,");
    }
}

Mutex::~Mutex() {
    int rt = pthread_mutex_destroy(&m_mutex);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_destroy failed, rt=" << rt;
    }
}

void Mutex::lock() {
    int rt = pthread_mutex_lock(&m_mutex);

    // 互斥锁被设置了 robust
    if (rt == EOWNERDEAD) {
        // Declare the state protected by MUTEX as consistent.
        pthread_mutex_consistent(&m_mutex);
    }
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_lock failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_lock failed,");
    }
}

bool Mutex::trylock() {
    int rt = pthread_mutex_trylock(&m_mutex);

    // 出现EBUSY 互斥量未被锁上
    if (rt == EBUSY) {
        return false;
    }

    if (rt == 0) {
        return true;
    }

    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_trylock failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_trylock failed,");
    }
    return false;
}

void Mutex::unlock() {
    int rt = pthread_mutex_unlock(&m_mutex);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_unlock failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_unlock failed,");
    }
}

bool Mutex::timed_lock(int sec, int msec) {
    timespec ts;
    ts.tv_sec = sec;
    ts.tv_nsec = msec;

    int rt = pthread_mutex_timedlock(&m_mutex, &ts);

    if (rt) {
        // 超时
        return false;
    }
    return true;
}

/******************************************************************
 *  递归锁
 * ***************************************************************/

recursive_mutex::recursive_mutex() {
    // 如果定义了 PTHREAD_MUTEX_RECURSIVE  采用pthread支持的递归锁
    int rt;
    pthread_mutexattr_t attr;

    rt = pthread_mutexattr_init(&attr);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_unlock failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_unlock failed,");
    }

    rt = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_unlock failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_unlock failed,");
    }

    rt = pthread_mutex_init(&m_mutex, &attr);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_unlock failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_unlock failed,");
    }
    
    rt = pthread_mutexattr_destroy(&attr);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_unlock failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_unlock failed,");
    }
}
recursive_mutex::~recursive_mutex() {
    int rt = pthread_mutex_destroy(&m_mutex);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_destroy failed, rt=" << rt;
    }
}

void recursive_mutex::lock() {
    int rt = pthread_mutex_lock(&m_mutex);

    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_lock failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_lock failed,");
    }
}
bool recursive_mutex::trylock() {
    int rt = pthread_mutex_trylock(&m_mutex);

    // 出现EBUSY 互斥量未被锁上
    if (rt == EBUSY) {
        return false;
    }

    if (rt == 0) {
        return true;
    }

    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_trylock failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_trylock failed,");
    }
    return false;
}

void recursive_mutex::unlock() {
    int rt = pthread_mutex_unlock(&m_mutex);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_unlock failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_unlock failed,");
    }
}

}  // namespace ppcode