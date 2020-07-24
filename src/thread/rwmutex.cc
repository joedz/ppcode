#include "rwmutex.h"

#include "../log.h"
#include "time.h"

namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();

RWMutex::RWMutex() {
    int rt = pthread_rwlock_init(&m_rwmutex, nullptr);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_rwlock_init failed, rt=" << rt;
//        throw std::logic_error("pthread_rwlock_init failed");
    }
}

RWMutex::~RWMutex() {
    int rt = pthread_rwlock_destroy(&m_rwmutex);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_rwlock_destroy failed, rt=" << rt;
    }
}

void RWMutex::rdlock() {
    int rt = pthread_rwlock_rdlock(&m_rwmutex);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_rwlock_rdlock failed, rt=" << rt;
 //       throw std::logic_error("pthread_rwlock_rdlock failed");
    }
}

void RWMutex::wrlock() {
    int rt = pthread_rwlock_wrlock(&m_rwmutex);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_rwlock_wrlock failed, rt=" << rt;
//        throw std::logic_error("pthread_rwlock_wrlock failed");
    }
}

bool RWMutex::try_rdlock() {
    int rt = pthread_rwlock_tryrdlock(&m_rwmutex);
    if (rt == EBUSY) {
        return false;
    }
    if (rt == 0) {
        return true;
    }
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_rwlock_tryrdlock failed, rt=" << rt;
 //       throw std::logic_error("pthread_rwlock_tryrdlock failed");
    }
    return false;
}

bool RWMutex::try_wrlock() {
    int rt = pthread_rwlock_trywrlock(&m_rwmutex);
    if (rt == EBUSY) {
        return false;
    } else if (rt == 0) {
        return true;
    }
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_rwlock_trywrlock failed, rt=" << rt;
 //       throw std::logic_error("pthread_rwlock_trywrlock failed");
    }
    return false;
}

void RWMutex::unlock() {
    int rt = pthread_rwlock_unlock(&m_rwmutex);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_rwlock_unlock failed, rt=" << rt;
 //       throw std::logic_error("pthread_rwlock_unlock failed");
    }
}

bool RWMutex::time_rdlock(int sec, int msec) {
    timespec ts;
    ts.tv_nsec = msec;
    ts.tv_sec = sec;
    int rt = pthread_rwlock_timedrdlock(&m_rwmutex, &ts);
    if (rt == 0) {
        return true;
    }
    // 超时
    if (rt == ETIMEDOUT) {
        return false;
    }
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_rwlock_timedrdlock failed, rt=" << rt;
 //       throw std::logic_error("pthread_rwlock_timedrdlock failed");
    }
    return false;
}
bool RWMutex::time_wrlock(int sec, int msec) {
    timespec ts;
    ts.tv_nsec = msec;
    ts.tv_sec = sec;
    int rt = pthread_rwlock_timedwrlock(&m_rwmutex, &ts);
    if (rt == 0) {
        return true;
    }
    // 超时
    if (rt == ETIMEDOUT) {
        return false;
    }
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_rwlock_timedwrlock failed, rt=" << rt;
 //       throw std::logic_error("pthread_rwlock_timedwrlock failed");
    }
    return false;
}

}  // namespace ppcode