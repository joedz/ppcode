#include "semaphore.h"
#include "../log.h"


namespace ppcode
{

static Logger::ptr g_logger = LOG_ROOT();

Semaphore::Semaphore(uint32_t count) {
    int rt = sem_init(&m_semaphore, 0, count);
    if(rt) {
         LOG_ERROR(g_logger) << "pthread_cond_init failed, rt=" << rt;
            throw std::logic_error("pthread_cond_init failed,");
    }
}

Semaphore::~Semaphore() {
    sem_destroy(&m_semaphore);
}

void Semaphore::wait() {
    int rt = sem_wait(&m_semaphore);
    if(rt) {
         LOG_ERROR(g_logger) << "pthread_cond_init failed, rt=" << rt;
        throw std::logic_error("pthread_cond_init failed,");
    }
}

void Semaphore::notify() {
    int rt = sem_post(&m_semaphore);
    if(rt) {
        LOG_ERROR(g_logger) << "pthread_cond_init failed, rt=" << rt;
        throw std::logic_error("pthread_cond_init failed,");
    }
}


}