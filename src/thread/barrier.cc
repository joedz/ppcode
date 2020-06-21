#include "barrier.h"

#include <pthread.h>

#include "../log.h"

namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();

Barrier::Barrier(int count) {
    int rt = pthread_barrier_init(&m_barrier, nullptr, count);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_barrier_init failed, rt=" << rt;
        throw std::logic_error("pthread_barrier_init failed");
    }
}

Barrier::~Barrier() {
    int rt = pthread_barrier_destroy(&m_barrier);
    if (rt) {
        LOG_ERROR(g_logger) << "pthread_barrier_destroy failed, rt=" << rt;
    }
}

bool Barrier::wait() {
    int rt;
    do {
        rt = pthread_barrier_wait(&m_barrier);
    } while (rt == EINTR);

    if (rt == PTHREAD_BARRIER_SERIAL_THREAD) {
        return true;
    }

    if (rt) {
        LOG_ERROR(g_logger) << "pthread_barrier_wait failed, rt=" << rt;
        throw std::logic_error("pthread_barrier_wait failed");
    }
    return false;
}

}  // namespace ppcode