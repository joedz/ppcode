#include "spinlock.h"
#include "../log.h"

namespace ppcode{

static Logger::ptr g_logger = LOG_ROOT();

Spinlock::Spinlock(){
    int rt = pthread_spin_init(&m_spinlock, 0);
    if(rt) {
        LOG_ERROR(g_logger) << "pthread_rwlock_timedrdlock failed, rt=" << rt;
        throw std::logic_error("pthread_rwlock_timedrdlock failed");
    }
}
Spinlock::~Spinlock(){
    int rt = pthread_spin_destroy(&m_spinlock);
    if(rt){
        LOG_ERROR(g_logger) << "pthread_spin_destroy failed, rt=" << rt;
        throw std::logic_error("pthread_spin_destroy failed");
    }
}

void Spinlock::lock(){
    int rt = pthread_spin_lock(&m_spinlock);
    if(rt == EDEADLK){
        LOG_DEBUG(g_logger) << "pthread_spin_lock Repeated locking";
        return;
    }
    if(rt){
        LOG_ERROR(g_logger) << "pthread_spin_lock failed, rt=" << rt;
        throw std::logic_error("pthread_spin_lock failed");
    }

}
void Spinlock::unlock(){
    int rt = pthread_spin_unlock(&m_spinlock);
    if(rt){
        LOG_ERROR(g_logger) << "pthread_spin_unlock failed, rt=" << rt;
        throw std::logic_error("pthread_spin_unlock failed");
    }
}

void Spinlock::trylock(){
    int rt = pthread_spin_trylock(&m_spinlock);
    if(rt){
        LOG_ERROR(g_logger) << "pthread_spin_trylock failed, rt=" << rt;
        throw std::logic_error("pthread_spin_trylock failed");
    }
}
    
} // namespace ppcode

