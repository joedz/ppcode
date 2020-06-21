#include <pthread.h>

#include "../log.h"
#include "condition_variable.h"

namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();

Condition_variable::Condition_variable() {
    int rt;
    rt = pthread_mutex_init(&m_mutex, nullptr);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_mutex_init failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_init failed,");
    }
    rt = pthread_cond_init(&m_cond, nullptr);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_cond_init failed, rt=" << rt;
        throw std::logic_error("pthread_cond_init failed,");
    }
}

Condition_variable::~Condition_variable() {
    int rt;
    if (!(rt = pthread_mutex_destroy(&m_mutex))) {
        LOG_ERROR(g_logger) << "pthread_mutex_destroy failed, rt=" << rt;
        throw std::logic_error("pthread_mutex_destroy failed,");
    }

    if (!(rt = pthread_cond_destroy(&m_cond))) {
        LOG_ERROR(g_logger) << "pthread_cond_destroy failed, rt=" << rt;
    }
}

void Condition_variable::wait(Mutex& lock) {
    // 条件变量
    // 进入后 将解锁 lock
    // 然后陷入等待 知道被notify

    pthread_mutex_t* the_mutex = lock.native_handle();
    int rt;
    do {
        rt = pthread_cond_wait(&m_cond, the_mutex);
    } while (rt == EINTR);

    if (rt) {
        LOG_ERROR(g_logger) << "pthread_cond_wait failed, rt=" << rt;
        throw std::logic_error("pthread_cond_wait failed,");
    }
}

void Condition_variable::timeWait(Mutex& lock, int sec, int mesc) {
    pthread_mutex_t* the_mutex = lock.native_handle();
    timespec ts;
    ts.tv_sec = sec;
    ts.tv_nsec = mesc;

    // 时间问题
    int rt;
    do {
        rt = pthread_cond_timedwait(&m_cond, the_mutex, &ts);
    } while (rt == EINTR);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_cond_timedwait failed, rt=" << rt;
        throw std::logic_error("pthread_cond_timedwait failed,");
    }
}

void Condition_variable::notify_one() {
    int rt = pthread_cond_signal(&m_cond);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_cond_signal failed, rt=" << rt;
        throw std::logic_error("pthread_cond_signal failed,");
    }
}

void Condition_variable::notify_all() {
    int rt = pthread_cond_broadcast(&m_cond);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_cond_broadcast failed, rt=" << rt;
        throw std::logic_error("pthread_cond_broadcast failed,");
    }
}

}  // namespace ppcode